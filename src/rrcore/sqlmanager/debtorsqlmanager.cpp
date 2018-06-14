#include "debtorsqlmanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

#include "singletons/userprofile.h"

DebtorSqlManager::DebtorSqlManager(QSqlDatabase connection) :
    AbstractSqlManager(connection)
{

}

QueryResult DebtorSqlManager::execute(const QueryRequest &request)
{
    QueryResult result(request);

    try {
        if (request.command() == "add_debtor")
            addDebtor(request, result);
        else if (request.command() == "undo_add_debtor")
            undoAddDebtor(request, result);
        else if (request.command() == "view_debtors")
            viewDebtors(request, result);
        else if (request.command() == "remove_debtor")
            removeDebtor(request, result);
        else if (request.command() == "undo_remove_debtor")
            undoRemoveDebtor(request, result);
        else
            throw DatabaseException(DatabaseException::RRErrorCode::CommandNotFound, QString("Command not found: %1").arg(request.command()));

        result.setSuccessful(true);
    } catch (DatabaseException &e) {
        result.setSuccessful(false);
        result.setErrorCode(e.code());
        result.setErrorMessage(e.message());
        result.setErrorUserMessage(e.userMessage());
    }

    return result;
}

void DebtorSqlManager::addDebtor(const QueryRequest &request, QueryResult &result)
{
    const QVariantMap &params = request.params();
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    const QVariantList debtTransactions = params.value("debt_transactions").toList();
    int noteId = 0;
    int debtorId = 0;
    int clientId = 0;
    QVector<int> debtTransactionNoteIds;
    QSqlQuery q(connection());

    try {
        if (!DatabaseUtils::beginTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::BeginTransactionFailed, q.lastError().text(), "Failed to start transation.");

        auto addNote = [&q, &currentDateTime](const QString &note) {
            if (note.trimmed().isEmpty())
                return 0;

            // Add note
            q.prepare("INSERT INTO note (note, table_name, created, last_edited, user_id) VALUES ("
                      ":note, :table_name, :created, :last_edited, :user_id)");
            q.bindValue(":note", note.isNull() ? QVariant(QVariant::String) : note);
            q.bindValue(":table_name", "debtor");
            q.bindValue(":created", currentDateTime);
            q.bindValue(":last_edited", currentDateTime);
            q.bindValue(":user_id", UserProfile::instance().userId());

            if (!q.exec())
                throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, q.lastError().text(), "Failed to insert into note table.");

            return q.lastInsertId().toInt();
        };

        noteId = addNote(params.value("note").toString());

        // Add notes for each debt transaction
        for (int i = 0; i < debtTransactions.count(); ++i)
            debtTransactionNoteIds.append(addNote(debtTransactions.at(i).toMap().value("note").toString()));

        if (debtTransactions.count() != debtTransactionNoteIds.count())
            throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, "",
                                    "Failed to match note ID count with transaction count.");

        // Add client
        q.prepare("INSERT INTO client (first_name, last_name, preferred_name, phone_number, address, note_id, archived, created, last_edited, "
                  "user_id) VALUES (:first_name, :last_name, :preferred_name, :phone_number, :address, :note_id, :archived, :created, "
                  ":last_edited, :user_id)");
        q.bindValue(":first_name", params.value("first_name"));
        q.bindValue(":last_name", params.value("last_name"));
        q.bindValue(":preferred_name", params.value("preferred_name"));
        q.bindValue(":phone_number", params.value("phone_number"));
        q.bindValue(":address", params.value("address"));
        q.bindValue(":note_id", noteId == 0 ? QVariant(QVariant::Int) : noteId);
        q.bindValue(":archived", false);
        q.bindValue(":created", currentDateTime);
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());

        if (!q.exec()) {
            if (q.lastError().number() == int(DatabaseException::MySqlErrorCode::DuplicateEntryError))
                throw DatabaseException(DatabaseException::RRErrorCode::DuplicateEntryFailure, q.lastError().text(),
                                        "Failed to insert client because client already exists.");
            else
                throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, q.lastError().text(),
                                        "Failed to insert into client table.");
        }

        clientId = q.lastInsertId().toInt();

        // Add debtor
        q.prepare("INSERT INTO debtor (client_id, note_id, archived, created, last_edited, user_id) VALUES ("
                  ":client_id, :note_id, :archived, :created, :last_edited, :user_id)");
        q.bindValue(":client_id", clientId);
        q.bindValue(":note_id", noteId == 0 ? QVariant(QVariant::Int) : noteId);
        q.bindValue(":archived", false);
        q.bindValue(":created", currentDateTime);
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, q.lastError().text(), "Failed to insert into debtor table.");

        debtorId = q.lastInsertId().toInt();

        // Add debt transactions
        for (int i = 0; i < debtTransactions.count(); ++i) {
            q.prepare("INSERT INTO debt_transaction (debtor_id, transaction_table, transaction_id, note_id, archived, created, last_edited, "
                      "user_id) VALUES (:debtor_id, :transaction_table, :transaction_id, :note_id, :archived, "
                      ":created, :last_edited, :user_id)");
            q.bindValue(":debtor_id", debtorId);
            q.bindValue(":transaction_table", "debtor");
            q.bindValue(":transaction_id", 0);
            q.bindValue(":note_id", debtTransactionNoteIds.at(i) == 0 ? QVariant(QVariant::Int) : debtTransactionNoteIds.at(i));
            q.bindValue(":archived", false);
            q.bindValue(":created", currentDateTime);
            q.bindValue(":last_edited", currentDateTime);
            q.bindValue(":user_id", UserProfile::instance().userId());

            if (!q.exec())
                throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, q.lastError().text(),
                                        "Failed to insert into debt transaction table.");

            const int debtTransactionId = q.lastInsertId().toInt();
            const double totalDebt = debtTransactions.at(i).toMap().value("total_debt").toDouble();
            const QVariantList debtPaymentList = debtTransactions.at(i).toMap().value("debt_payments").toList();
            const QDateTime dueDateTime = debtTransactions.at(i).toMap().value("due_date_time").toDateTime();
            double newDebt = totalDebt;

            for (const QVariant &v : debtPaymentList) {
                const double amountPaid = v.toMap().value("amount").toDouble();
                const QString note = v.toMap().value("note").toString();

                const int noteId = addNote(note);

                // Add debt payment
                q.prepare("INSERT INTO debt_payment (debt_transaction_id, total_amount, amount_paid, balance, currency, "
                          "due_date, note_id, archived, created, last_edited, user_id) VALUES (:debt_transaction_id, "
                          ":total_amount, :amount_paid, :balance, :currency, :due_date, :note_id, :archived, "
                          ":created, :last_edited, :user_id)");
                q.bindValue(":debt_transaction_id", debtTransactionId);
                q.bindValue(":total_amount", newDebt);
                q.bindValue(":amount_paid", amountPaid);
                q.bindValue(":balance", newDebt - amountPaid);
                q.bindValue(":currency", "NGN");
                q.bindValue(":due_date", dueDateTime);
                q.bindValue(":note_id", noteId == 0 ? QVariant(QVariant::Int) : noteId);
                q.bindValue(":archived", false);
                q.bindValue(":created", currentDateTime);
                q.bindValue(":last_edited", currentDateTime);
                q.bindValue(":user_id", UserProfile::instance().userId());

                if (!q.exec())
                    throw DatabaseException(DatabaseException::RRErrorCode::AddDebtorFailure, q.lastError().text(),
                                            "Failed to insert into debt payment table.");

                debtPaymentList.at(i).toDouble();
                newDebt -= amountPaid;
            }

            if (newDebt < 0)
                throw DatabaseException(DatabaseException::RRErrorCode::AmountOverpaid, q.lastError().text(),
                                        "Total amount paid is greater than total debt.");

            if (dueDateTime <= QDateTime::currentDateTime())
                throw DatabaseException(DatabaseException::RRErrorCode::InvalidDueDate, q.lastError().text(),
                                        "Due date is earlier than the current date.");

            QVariantMap outcome;
            outcome.insert("client_id", clientId);
            outcome.insert("debtor_id", debtorId);
            outcome.insert("debt_transaction_id", debtTransactionId);
            result.setOutcome(outcome);
        }

        if (!DatabaseUtils::commitTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::CommitTransationFailed, q.lastError().text(), "Failed to commit.");
    } catch (DatabaseException &) {
        if (!DatabaseUtils::rollbackTransaction(q))
            qCritical("Failed to rollback failed transaction! %s", q.lastError().text().toStdString().c_str());

        throw;
    }
}

void DebtorSqlManager::undoAddDebtor(const QueryRequest &request, QueryResult &result)
{
    Q_UNUSED(result)

    const QVariantMap &params = request.params();
    const QDateTime currentDateTime = QDateTime::currentDateTime();

    qDebug() << ">>>" << params.value("outcome").toMap().value("debtor_id").toInt() << params.value("outcome").toMap().value("debt_transaction_id").toInt();

    QSqlQuery q(connection());

    try {
        if (!DatabaseUtils::beginTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::BeginTransactionFailed, q.lastError().text(), "Failed to start transation.");

        // Remove debtor
        q.prepare("UPDATE debtor SET archived = 1, last_edited = :last_edited, user_id = :user_id WHERE id = :debtor_id");
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());
        q.bindValue(":debtor_id", params.value("outcome").toMap().value("debtor_id"));

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::UndoAddDebtorFailure, q.lastError().text(),
                                    "Failed to archive debtor.");

        // Remove debt transaction
        q.prepare("UPDATE debt_transaction SET archived = 1, last_edited = :last_edited, user_id = :user_id "
                  "WHERE id = :debt_transaction_id");
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());
        q.bindValue(":debt_transaction_id", params.value("outcome").toMap().value("debt_transaction_id"));

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::UndoAddDebtorFailure, q.lastError().text(),
                                    "Failed to archive debt transaction.");

        // Remove debt payment for debt transaction ID
        q.prepare("UPDATE debt_payment SET archived = 1, last_edited = :last_edited, user_id = :user_id "
                  "WHERE debt_transaction_id = :debt_transaction_id");
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());
        q.bindValue(":debt_transaction_id", params.value("outcome").toMap().value("debt_transaction_id"));

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::UndoAddDebtorFailure, q.lastError().text(),
                                    "Failed to archive debt payments.");

        if (!DatabaseUtils::commitTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::CommitTransationFailed, q.lastError().text(), "Failed to commit.");
    } catch (DatabaseException &) {
        if (!DatabaseUtils::rollbackTransaction(q))
            qCritical("Failed to rollback failed transaction! %s", q.lastError().text().toStdString().c_str());

        throw;
    }
}

void DebtorSqlManager::viewDebtors(const QueryRequest &request, QueryResult &result)
{
    const QVariantMap &params = request.params();
    QSqlQuery q(connection());

    /*
      SELECT debtor.client_id, client.preferred_name AS name,
      (SELECT SUM(debt_payment.balance) FROM debt_payment
      INNER JOIN debt_transaction ON debt_transaction.id = debt_payment.debt_transaction_id
      INNER JOIN debtor ON debt_transaction.debtor_id = debtor.id
      WHERE debt_payment.debt_transaction_id IN
     (SELECT debt_transaction.id FROM debt_transaction WHERE debt_transaction.debtor_id = debtor.id AND debt_transaction.archived = 0)
     AND debt_payment.archived = 0 ORDER BY debt_payment.last_edited DESC LIMIT 1) AS balance,
      note.note AS note, debtor.created, debtor.last_edited, debtor.user_id, user.user
      FROM debtor
      INNER JOIN client ON client.id = debtor.client_id
      LEFT JOIN user on user.id = debtor.user_id
      LEFT JOIN note ON debtor.note_id = note.id
      WHERE debtor.archived = 0;
     */
    try {
        // Get total balance for each debtor
        q.prepare("SELECT debtor.client_id, client.preferred_name AS name, "
                  "(SELECT SUM(debt_payment.balance) FROM debt_payment "
                  "INNER JOIN debt_transaction ON debt_transaction.id = debt_payment.debt_transaction_id "
                  "INNER JOIN debtor ON debt_transaction.debtor_id = debtor.id "
                  "WHERE debt_payment.debt_transaction_id IN "
                  "(SELECT debt_transaction.id FROM debt_transaction WHERE debt_transaction.debtor_id = debtor.id AND debt_transaction.archived = 0) "
                  "AND debt_payment.archived = 0 ORDER BY debt_payment.last_edited DESC LIMIT 1) AS total_balance, "
                  "note.note AS note, debtor.created, debtor.last_edited, debtor.user_id, user.user "
                  "FROM debtor "
                  "INNER JOIN client ON client.id = debtor.client_id "
                  "LEFT JOIN user on user.id = debtor.user_id "
                  "LEFT JOIN note ON debtor.note_id = note.id "
                  "WHERE debtor.archived = :archived");
        q.bindValue(":archived", params.value("archived", false), QSql::Out);

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::ViewDebtorsFailure, q.lastError().text(), "Failed to fetch debtors.");

        QVariantList debtors;
        while (q.next()) {
            debtors.append(recordToMap(q.record()));
        }

        result.setOutcome(QVariantMap { { "debtors", debtors }, { "record_count", debtors.count() } });
    } catch (DatabaseException &) {
        throw;
    }
}

void DebtorSqlManager::removeDebtor(const QueryRequest &request, QueryResult &result)
{
    const QVariantMap params = request.params();
    const QDateTime currentDateTime = QDateTime::currentDateTime();

    QSqlQuery q(connection());

    try {
        if (params.value("debtor_id").toInt() <= 0)
            throw DatabaseException(DatabaseException::RRErrorCode::InvalidArguments, QString(), "Debtor ID is invalid.");
        if (!DatabaseUtils::beginTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::BeginTransactionFailed, q.lastError().text(), "Failed to start transation.");

        QSqlQuery q(connection());
        q.prepare("UPDATE debtor SET archived = 1, last_edited = :last_edited, user_id = :user_id WHERE id = :debtor_id");
        q.bindValue(":debtor_id", params.value("debtor_id"));
        q.bindValue(":last_edited", currentDateTime);
        q.bindValue(":user_id", UserProfile::instance().userId());

        if (!q.exec())
            throw DatabaseException(DatabaseException::RRErrorCode::RemoveStockItemFailed, q.lastError().text(), "Failed to remove debtor.");

        result.setOutcome( { { "debtor_id", q.value("debtor_id") } });

        if (!DatabaseUtils::commitTransaction(q))
            throw DatabaseException(DatabaseException::RRErrorCode::CommitTransationFailed, q.lastError().text(), "Failed to commit.");
    } catch (DatabaseException &) {
        if (!DatabaseUtils::rollbackTransaction(q))
            qCritical("Failed to rollback failed transaction! %s", q.lastError().text().toStdString().c_str());

        throw;
    }
}

void DebtorSqlManager::undoRemoveDebtor(const QueryRequest &request, QueryResult &result)
{
    Q_UNUSED(request)
    Q_UNUSED(result)
}
