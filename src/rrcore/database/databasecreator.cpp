#include "databasecreator.h"
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlDriver>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QDirIterator>
#include <QSettings>

#include "database/databaseexception.h"
#include "database/databaseutils.h"
#include "config/config.h"

const QString CONNECTION_NAME("databasecreator");

const QString INIT_SQL_FILE(":/sql/init.sql");
const QString PROCEDURE_DIR(":/sql/procedures");

//const QString CREATE_PROCEDURE_PATTERN("(?<=DELIMITER \\/\\/).*(?=\\/\\/ DELIMITER ;)");
const QString CREATE_PROCEDURE_PATTERN("(?<=###BEGIN###;).*(?=###END###;)");
const QString DATABASE_NAME_PATTERN("###DATABASENAME###");
const QString NO_COMMENT_OR_SPACE_PATTERN("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)");

DatabaseCreator::DatabaseCreator(QSqlDatabase connection) :
    m_connection(connection)
{
    if (!m_connection.isValid()) {
        if (!QSqlDatabase::contains(CONNECTION_NAME))
            m_connection = QSqlDatabase::addDatabase("QMYSQL", CONNECTION_NAME);
        else
            m_connection = QSqlDatabase::database(CONNECTION_NAME);

        // Disconnect and connect to 'mysql'
        if (m_connection.isOpen())
            m_connection.close();

        m_connection.setDatabaseName("mysql");
        m_connection.setHostName(Config::instance().hostName());
        m_connection.setPort(Config::instance().port());
        m_connection.setUserName(Config::instance().userName());
        m_connection.setPassword(Config::instance().password());
        m_connection.setConnectOptions("MYSQL_OPT_RECONNECT = 1");

        m_connection.open();
    }
}

void DatabaseCreator::executeSqlFile(const QString &fileName)
{
    if (fileName.trimmed().isEmpty())
        return;

    QFile file(fileName);
    if (QFileInfo(fileName).suffix() != "sql")
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("File '%1' is not a sql file").arg(fileName));
    if (QFileInfo(fileName).size() > 1024 * 50)
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("File '%1' is too large (larger than 50MB).").arg(fileName));
    if (!file.open(QFile::ReadOnly))
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("Failed to open '%1'").arg(fileName));

    QSqlQuery q(m_connection);
    QString sqlData = file.readAll();

    if (Config::instance().databaseName().toLower() == QStringLiteral("mysql"))
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed,
                                q.lastError().text(), "Database name cannot be mysql.");

    // Inject database name
    sqlData = sqlData.replace(QRegularExpression(DATABASE_NAME_PATTERN), Config::instance().databaseName());

    if(m_connection.driver()->hasFeature(QSqlDriver::Transactions)) {
        // Replace comments and tabs and new lines with space
        sqlData = sqlData.replace(QRegularExpression(NO_COMMENT_OR_SPACE_PATTERN,
                                                     QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption), " ");
        // Remove waste spaces
        sqlData = sqlData.trimmed();

        // Extract queries
        QStringList extractedQueries = sqlData.split(';', QString::SkipEmptyParts);

        // Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
        QRegularExpression re_transaction("\\bbegin.transaction.*", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression re_commit("\\bcommit.*", QRegularExpression::CaseInsensitiveOption);

        // Check if query file is already wrapped with a transaction
        bool isStartedWithTransaction = re_transaction.match(extractedQueries.at(0)).hasMatch();
        if(!isStartedWithTransaction)
            m_connection.transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.

        //Execute each individual queries
        for(const QString &s : extractedQueries) {
            if(re_transaction.match(s).hasMatch())    //<== detecting special query
                m_connection.transaction();
            else if(re_commit.match(s).hasMatch())    //<== detecting special query
                m_connection.commit();
            else {
                q.exec(s);                        //<== execute normal query
                if(q.lastError().type() != QSqlError::NoError) {
                    qInfo() << q.lastError().text();
                    m_connection.rollback();                    //<== rollback the transaction if there is any problem
                }
            }
        }
        if(!isStartedWithTransaction)
            m_connection.commit();          //<== ... completing of wrapping with transaction

        //Sql Driver doesn't supports transaction
    } else {
        //...so we need to remove special queries (`begin transaction` and `commit`)
        sqlData = sqlData.replace(QRegularExpression("(\\bbegin.transaction.*;|\\bcommit.*;|\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)",
                                                     QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption), " ");
        sqlData = sqlData.trimmed();

        //Execute each individual queries
        QStringList extractedQueries = sqlData.split(';', QString::SkipEmptyParts);
        foreach(const QString &s, extractedQueries) {
            q.exec(s);
            if(q.lastError().type() != QSqlError::NoError)
                qInfo() << q.lastError().text();
        }
    }
}

bool DatabaseCreator::start()
{
    try {
        QSettings settings;

        initDatabase();
        createProcedures();

        settings.setValue("is_first_time", false);
    } catch (DatabaseException &e) {
        qDebug() << "Exception caught:" << e.what();
        return false;
    }

    return true;
}

void DatabaseCreator::initDatabase()
{
    executeSqlFile(INIT_SQL_FILE);
}

void DatabaseCreator::createProcedures()
{
    QDirIterator iter(PROCEDURE_DIR);
    while (iter.hasNext()) {
        QFile file(iter.next());
        if (QFileInfo(file).suffix() != "sql")
            continue;
        if (!file.open(QFile::ReadOnly))
            throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed,
                                    file.errorString(),
                                    QStringLiteral("Failed to create procedures!"));

        executeStoredProcedures(QFileInfo(file).filePath());
    }
}

void DatabaseCreator::executeStoredProcedures(const QString &fileName)
{
    if (fileName.trimmed().isEmpty())
        return;

    QFile file(fileName);
    if (QFileInfo(fileName).suffix() != "sql")
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("File '%1' is not a sql file").arg(fileName));
    if (QFileInfo(fileName).size() > 1024 * 50)
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("File '%1' is too large (larger than 50MB).").arg(fileName));
    if (!file.open(QFile::ReadOnly))
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed, QString(),
                                QString("Failed to open '%1'").arg(fileName));

    // Drop procedures first
    QSqlQuery q(m_connection);
    while (file.bytesAvailable()) {
        QString sqlLine = file.readLine();
        if (sqlLine.trimmed().startsWith("DROP PROCEDURE")) {
            sqlLine = sqlLine.remove(';').remove('\n');
            if (!q.exec(sqlLine))
                throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed,
                                        q.lastError().text(),
                                        QString("Failed to execute query: %1").arg(sqlLine));
        }
    }

    // Now, create defined procedures
    file.seek(0);
    QString sqlData = file.readAll();

    if (Config::instance().databaseName().toLower() == QStringLiteral("mysql"))
        throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed,
                                q.lastError().text(), "Database name cannot be mysql.");

    // Inject database name
    sqlData = sqlData.replace(QRegularExpression(DATABASE_NAME_PATTERN), Config::instance().databaseName());

    // Replace comments and tabs and new lines with space
    sqlData = sqlData.replace(QRegularExpression(NO_COMMENT_OR_SPACE_PATTERN,
                                                 QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption), " ");
    // Remove waste spaces
    sqlData = sqlData.trimmed();

    QRegularExpression expression(CREATE_PROCEDURE_PATTERN,
                                  QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = expression.match(sqlData);
    if (match.hasMatch()) {
        for (int i = 0; i < match.capturedLength(); ++i) {
            if (!match.captured(i).trimmed().isEmpty()) {
                if (!q.exec(match.captured(i).trimmed()))
                    throw DatabaseException(DatabaseException::RRErrorCode::DatabaseInitializationFailed,
                                            q.lastError().text(),
                                            QString("Failed to execute query: %1").arg(match.captured(i).trimmed()));
            }
        }
    }
}
