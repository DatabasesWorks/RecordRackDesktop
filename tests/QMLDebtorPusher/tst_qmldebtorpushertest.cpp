#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>

#include "qmlapi/qmldebtorpusher.h"
#include "databaseclient.h"
#include "singletons/userprofile.h"

class QMLDebtorPusherTest : public QObject
{
    Q_OBJECT

public:
    QMLDebtorPusherTest();

private Q_SLOTS:
    void init();
    void cleanup();

    void testSetImageSource();
    void testSetFirstName();
    void testSetLastName();
    void testSetPreferredName();
    void testSetPhoneNumber();
    void testSetAddress();
    void testSetNote();
    void testAddDebtorWithNoPhoneNumber();
    void testAddDebtorWithNoPreferredName();

    // Long-running tests
    void testAddNewDebtor();
    void testAddSameDebtor();
    void testAddDebt();
    void testAddPayment();
    void testUndoAddNewDebtor();

private:
    QMLDebtorPusher *m_debtorPusher;
    DatabaseClient *m_client;
};

QMLDebtorPusherTest::QMLDebtorPusherTest()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.info=false"));
}

void QMLDebtorPusherTest::init()
{
    m_debtorPusher = new QMLDebtorPusher(this);
    m_client = new DatabaseClient;
}

void QMLDebtorPusherTest::cleanup()
{
    m_debtorPusher->deleteLater();
    delete m_client;
}

void QMLDebtorPusherTest::testSetImageSource()
{
    QSignalSpy imageSourceChangedSpy(m_debtorPusher, &QMLDebtorPusher::imageSourceChanged);

    QCOMPARE(m_debtorPusher->imageSource(), QString());

    m_debtorPusher->setImageSource("Source");

    QCOMPARE(m_debtorPusher->imageSource(), "Source");
    QCOMPARE(imageSourceChangedSpy.count(), 1);
    imageSourceChangedSpy.clear();

    m_debtorPusher->setImageSource("Source");
    QCOMPARE(imageSourceChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetFirstName()
{
    QSignalSpy firstNameChangedSpy(m_debtorPusher, &QMLDebtorPusher::firstNameChanged);

    QCOMPARE(m_debtorPusher->firstName(), QString());

    m_debtorPusher->setFirstName("First name");

    QCOMPARE(m_debtorPusher->firstName(), "First name");
    QCOMPARE(firstNameChangedSpy.count(), 1);
    firstNameChangedSpy.clear();

    m_debtorPusher->setImageSource("First name");
    QCOMPARE(firstNameChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetLastName()
{
    QSignalSpy lastNameChangedSpy(m_debtorPusher, &QMLDebtorPusher::lastNameChanged);

    QCOMPARE(m_debtorPusher->lastName(), QString());

    m_debtorPusher->setLastName("Last name");

    QCOMPARE(m_debtorPusher->lastName(), "Last name");
    QCOMPARE(lastNameChangedSpy.count(), 1);
    lastNameChangedSpy.clear();

    m_debtorPusher->setLastName("Last name");
    QCOMPARE(lastNameChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetPreferredName()
{
    QSignalSpy preferredNameChangedSpy(m_debtorPusher, &QMLDebtorPusher::preferredNameChanged);

    QCOMPARE(m_debtorPusher->preferredName(), QString());

    m_debtorPusher->setPreferredName("Preferred name");

    QCOMPARE(m_debtorPusher->preferredName(), "Preferred name");
    QCOMPARE(preferredNameChangedSpy.count(), 1);
    preferredNameChangedSpy.clear();

    m_debtorPusher->setLastName("Preferred name");
    QCOMPARE(preferredNameChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetPhoneNumber()
{
    QSignalSpy phoneNumberChangedSpy(m_debtorPusher, &QMLDebtorPusher::phoneNumberChanged);

    QCOMPARE(m_debtorPusher->phoneNumber(), QString());

    m_debtorPusher->setPhoneNumber("1234567890");

    QCOMPARE(m_debtorPusher->phoneNumber(), "1234567890");
    QCOMPARE(phoneNumberChangedSpy.count(), 1);
    phoneNumberChangedSpy.clear();

    m_debtorPusher->setPhoneNumber("1234567890");
    QCOMPARE(phoneNumberChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetAddress()
{
    QSignalSpy addressChangedSpy(m_debtorPusher, &QMLDebtorPusher::addressChanged);

    QCOMPARE(m_debtorPusher->address(), QString());

    m_debtorPusher->setAddress("1234 Address Street");

    QCOMPARE(m_debtorPusher->address(), "1234 Address Street");
    QCOMPARE(addressChangedSpy.count(), 1);
    addressChangedSpy.clear();

    m_debtorPusher->setPhoneNumber("1234 Address Street");
    QCOMPARE(addressChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testSetNote()
{
    QSignalSpy noteChangedSpy(m_debtorPusher, &QMLDebtorPusher::noteChanged);

    QCOMPARE(m_debtorPusher->note(), QString());

    m_debtorPusher->setNote("Note");

    QCOMPARE(m_debtorPusher->note(), "Note");
    QCOMPARE(noteChangedSpy.count(), 1);
    noteChangedSpy.clear();

    m_debtorPusher->setNote("Note");
    QCOMPARE(noteChangedSpy.count(), 0);
}

void QMLDebtorPusherTest::testAddDebtorWithNoPhoneNumber()
{
    QSignalSpy errorSpy(m_debtorPusher, &QMLDebtorPusher::error);

    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->setNote("Note");

    m_debtorPusher->push();
    QCOMPARE(errorSpy.count(), 1);
    const QList<QVariant> arguments = errorSpy.takeFirst();
    QVERIFY(arguments.at(0).toInt() == QMLDebtorPusher::NoPhoneNumberError);
}

void QMLDebtorPusherTest::testAddDebtorWithNoPreferredName()
{
    QSignalSpy errorSpy(m_debtorPusher, &QMLDebtorPusher::error);

    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->setNote("Note");

    m_debtorPusher->push();
    QCOMPARE(errorSpy.count(), 1);
    const QList<QVariant> arguments = errorSpy.takeFirst();
    QVERIFY(arguments.at(0).toInt() == QMLDebtorPusher::NoPreferredNameError);
}

void QMLDebtorPusherTest::testAddNewDebtor()
{
    QSignalSpy successSpy(m_debtorPusher, &QMLDebtorPusher::success);
    const QDateTime dueDateTime(QDateTime::currentDateTime().addDays(1));

    QVERIFY(m_client->initialize());

    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->setNote("Note");
    m_debtorPusher->addDebt(1234.56, dueDateTime, "Debt note");

    m_debtorPusher->push();
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(successSpy.count(), 1);


    /******************* DATABASE CHECKS ****************/
    QSqlQuery q(m_client->connection());
    q.prepare("SELECT id, first_name, last_name, preferred_name, phone_number, address, note_id, archived, created, last_edited, user_id FROM client");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    QCOMPARE(q.value("id").toInt(), 1);
    QCOMPARE(q.value("first_name").toString(), "First name");
    QCOMPARE(q.value("last_name").toString(), "Last name");
    QCOMPARE(q.value("preferred_name").toString(), "Preferred name");
    QCOMPARE(q.value("phone_number").toString(), "1234567890");
    QCOMPARE(q.value("address").toString(), "1234 Address Street");
    QCOMPARE(q.value("archived").toBool(), false);
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());

    q.prepare("SELECT id, client_id, note_id, archived, created, last_edited, user_id FROM debtor");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    QCOMPARE(q.value("id").toInt(), 1);
    QCOMPARE(q.value("client_id").toInt(), 1);
    QCOMPARE(q.value("note_id").toInt(), 1);
    QCOMPARE(q.value("archived").toBool(), false);
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());

    q.prepare("SELECT id, note, table_name, created, last_edited, user_id FROM note");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    QCOMPARE(q.value("id").toInt(), 1);
    QCOMPARE(q.value("note").toString(), "Note");
    QCOMPARE(q.value("table_name").toString(), "debtor");
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());
    /****************************************************/
}

void QMLDebtorPusherTest::testAddSameDebtor()
{
    QSignalSpy successSpy(m_debtorPusher, &QMLDebtorPusher::success);
    QSignalSpy errorSpy(m_debtorPusher, &QMLDebtorPusher::error);
    QSignalSpy busyChangedSpy(m_debtorPusher, &QMLDebtorPusher::busyChanged);
    const QDateTime dueDateTime(QDateTime::currentDateTime().addDays(1));

    QVERIFY(m_client->initialize());

    // Add a debtor.
    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->setNote("Note");
    m_debtorPusher->addDebt(1234.56, dueDateTime);
    m_debtorPusher->push();
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(busyChangedSpy.count(), 2);
    busyChangedSpy.clear();
    QCOMPARE(successSpy.count(), 1);
    successSpy.clear();
    QCOMPARE(errorSpy.count(), 0);

    // STEP: Re-add debt since debts are always cleared after DebtorPusher::push() is called.
    m_debtorPusher->addDebt(1234.56, dueDateTime);

    // STEP: Try to add the same debtor again.
    m_debtorPusher->push();
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(busyChangedSpy.count(), 2);
    busyChangedSpy.clear();
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().first().value<QMLDebtorPusher::ErrorCode>(), QMLDebtorPusher::DuplicateEntryError);
    errorSpy.clear();
    QCOMPARE(successSpy.count(), 0);

    QSqlQuery q(m_client->connection());
    // STEP: Verify client table was updated properly.
    q.prepare("SELECT id, first_name, last_name, preferred_name, phone_number, address, note_id, created, last_edited, user_id FROM client");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 1);

    // STEP: Verify debtor table was updated properly.
    q.prepare("SELECT id, client_id, note_id, created, last_edited, user_id FROM debtor");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 1);

    // STEP: Verify note table was updated properly.
    q.prepare("SELECT id, note, table_name, created, last_edited, user_id FROM note");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 1);
}

void QMLDebtorPusherTest::testAddDebt()
{
    QSignalSpy successSpy(m_debtorPusher, &QMLDebtorPusher::success);
    const QDateTime dueDateTime(QDateTime::currentDateTime().addDays(1));

    QVERIFY(m_client->initialize());

    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->addDebt(1234.56, dueDateTime, "Debt note");
    m_debtorPusher->push();
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(successSpy.count(), 1);

    /******************* DATABASE CHECKS ****************/
    QSqlQuery q(m_client->connection());
    q.prepare("SELECT id, debtor_id, transaction_table, note_id, archived, created, last_edited, user_id FROM debt_transaction");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    QCOMPARE(q.size(), 1);
    QCOMPARE(q.value("id").toInt(), 1);
    QCOMPARE(q.value("debtor_id").toInt(), 1);
    QCOMPARE(q.value("transaction_table").toString(), "debtor");
    QVERIFY(q.value("note_id").toInt() > 0);
    QCOMPARE(q.value("archived").toBool(), false);
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());

    q.prepare("SELECT id, note, table_name, created, last_edited, user_id FROM note WHERE note = 'Debt note'");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 1);
    /****************************************************/
}

void QMLDebtorPusherTest::testAddPayment()
{
    QSignalSpy successSpy(m_debtorPusher, &QMLDebtorPusher::success);
    QSignalSpy errorSpy(m_debtorPusher, &QMLDebtorPusher::error);
    const QDate dueDate(QDate::currentDate().addDays(1));

    // NOTE: Must be called before any database operations
    QVERIFY(m_client->initialize());

    // STEP: Add a new debtor.
    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->addDebt(1234.56, QDateTime(dueDate));
    m_debtorPusher->addPayment(0, 4.56, "Payment note");
    m_debtorPusher->push();
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(successSpy.takeFirst().first().value<QMLDebtorPusher::SuccessCode>(), QMLDebtorPusher::AddDebtorSuccess);
    successSpy.clear();

    QSqlQuery q(m_client->connection());
    q.prepare("SELECT id, debtor_id, transaction_table, note_id, archived, created, last_edited, user_id FROM debt_transaction");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    QCOMPARE(q.size(), 1);
    QCOMPARE(q.value("id").toInt(), 1);
    QCOMPARE(q.value("debtor_id").toInt(), 1);
    QCOMPARE(q.value("transaction_table").toString(), "debtor");
    QCOMPARE(q.value("note_id"), QVariant(QVariant::Int));
    QCOMPARE(q.value("archived").toBool(), false);
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());

    q.prepare("SELECT id, debt_transaction_id, total_amount, amount_paid, balance, due_date, note_id, archived, "
              "created, last_edited, user_id FROM debt_payment");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 2);
    QVERIFY(q.last());
    QCOMPARE(q.value("id").toInt(), 2);
    QCOMPARE(q.value("debt_transaction_id").toInt(), 1);
    QCOMPARE(q.value("total_amount").toDouble(), 1234.56);
    QCOMPARE(q.value("amount_paid").toDouble(), 4.56);
    QCOMPARE(q.value("balance").toDouble(), 1234.56 - 4.56);
    QCOMPARE(q.value("due_date").toDate(), dueDate);
    QVERIFY(q.value("note_id").toInt() > 0);
    QCOMPARE(q.value("archived").toBool(), false);
    QCOMPARE(q.value("created").toDateTime(), q.value("last_edited").toDateTime());
    QCOMPARE(q.value("user_id").toInt(), UserProfile::instance().userId());

    q.prepare("SELECT id, note, table_name, created, last_edited, user_id FROM note WHERE note = 'Payment note'");
    QVERIFY(q.exec());
    QCOMPARE(q.size(), 1);
}

void QMLDebtorPusherTest::testUndoAddNewDebtor()
{
    QSignalSpy successSpy(m_debtorPusher, &QMLDebtorPusher::success);
    QSignalSpy errorSpy(m_debtorPusher, &QMLDebtorPusher::error);
    QSignalSpy busyChangedSpy(m_debtorPusher, &QMLDebtorPusher::busyChanged);
    const QDateTime dueDateTime(QDateTime::currentDateTime().addDays(1));

    QVERIFY(m_client->initialize());

    m_debtorPusher->setImageSource("image/source");
    m_debtorPusher->setFirstName("First name");
    m_debtorPusher->setLastName("Last name");
    m_debtorPusher->setPreferredName("Preferred name");
    m_debtorPusher->setPhoneNumber("1234567890");
    m_debtorPusher->setAddress("1234 Address Street");
    m_debtorPusher->addDebt(1234.56, dueDateTime);

    m_debtorPusher->push();
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(busyChangedSpy.count(), 2);
    busyChangedSpy.clear();
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(successSpy.takeFirst().first().value<QMLDebtorPusher::SuccessCode>(), QMLDebtorPusher::AddDebtorSuccess);
    successSpy.clear();
    QCOMPARE(errorSpy.count(), 0);

    m_debtorPusher->undoLastCommit();
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(QTest::qWaitFor([&]() { return !m_debtorPusher->isBusy(); }, 2000));
    QCOMPARE(busyChangedSpy.count(), 2);
    busyChangedSpy.clear();
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(successSpy.takeFirst().first().value<QMLDebtorPusher::SuccessCode>(), QMLDebtorPusher::UndoAddDebtorSuccess);

    /******************* DATABASE CHECKS ****************/
    QSqlQuery q(m_client->connection());
    q.prepare("SELECT id FROM debtor WHERE id = 1 AND archived = 1");
    QVERIFY(q.exec());
    QVERIFY(q.first());

    q.prepare("SELECT id FROM debt_transaction WHERE debtor_id = 1 AND archived = 1");
    QVERIFY(q.exec());
    QVERIFY(q.first());
    /****************************************************/
}

QTEST_MAIN(QMLDebtorPusherTest)

#include "tst_qmldebtorpushertest.moc"