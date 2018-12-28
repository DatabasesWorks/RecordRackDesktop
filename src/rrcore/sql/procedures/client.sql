USE ###DATABASENAME###;

###BEGIN###;
CREATE PROCEDURE ViewClients (
    IN iFilterColumn VARCHAR(100),
    IN iFilterText VARCHAR(100),
    IN iArchived TINYINT
    )
BEGIN
    IF LOWER(iFilterColumn) = 'preferred_name' THEN
        SELECT id, preferred_name, phone_number FROM client
        WHERE client.archived = IFNULL(iArchived, FALSE) AND client.preferred_name LIKE CONCAT('%', IFNULL(iFilterText, ''), '%');
    ELSEIF LOWER(iFilterColumn) = 'phone_number' THEN
        SELECT id, preferred_name, phone_number FROM client
        WHERE client.archived = IFNULL(iArchived, FALSE) AND client.phone_number LIKE CONCAT('%', IFNULL(iFilterText, ''), '%');
    ELSE
        SELECT id, preferred_name, phone_number FROM client
        WHERE client.archived = IFNULL(iArchived, FALSE);
   END IF;
END;
###END###;

###BEGIN###;
CREATE PROCEDURE AddClient (
	IN iFirstName VARCHAR(100),
    IN iLastName VARCHAR(100),
    IN iPreferredName VARCHAR(100),
    IN iPhoneNumber VARCHAR(20),
    IN iAddress VARCHAR(100),
    IN iNoteId INTEGER,
    IN iUserId INTEGER
    )
BEGIN
	INSERT IGNORE INTO client (first_name, last_name, preferred_name, phone_number, address, note_id, archived, created, last_edited, user_id)
		VALUES (iFirstName, iLastName, iPreferredName, iPhoneNumber, iAddress, iNoteId, CURRENT_TIMESTAMP(), CURRENT_TIMESTAMP(), iUserId);

    IF LAST_INSERT_ID() > 0 THEN
		SELECT LAST_INSERT_ID();
	ELSE
		SELECT id FROM client WHERE phone_number = iPhoneNumber;
	END IF;
END;
###END###;

###BEGIN###;
CREATE PROCEDURE AddClientQuick (
    IN iPreferredName VARCHAR(100),
    IN iPhoneNumber VARCHAR(20),
    IN iUserId INTEGER
    )
BEGIN
    INSERT INTO client (preferred_name, phone_number, archived, created, last_edited, user_id)
        SELECT * FROM (SELECT iPreferredName, iPhoneNumber, FALSE, CURRENT_TIMESTAMP() AS created, CURRENT_TIMESTAMP() AS last_edited, iUserId) AS tmp
        WHERE NOT EXISTS (
            SELECT phone_number FROM client WHERE phone_number = iPhoneNumber
        ) LIMIT 1;

    SELECT id FROM client WHERE phone_number = iPhoneNumber;
END;
###END###;

###BEGIN###;
CREATE PROCEDURE ArchiveClient (
	IN iClientId INTEGER,
    IN iUserId INTEGER
)
BEGIN
	UPDATE client SET archived = 1, last_edited = CURRENT_TIMESTAMP(), user_id = iUserId WHERE id = iClientId;
END;
###END###;

###BEGIN###;
CREATE PROCEDURE UpdateClient (
	IN iClientId INTEGER,
	IN iFirstName VARCHAR(100),
    IN iLastName VARCHAR(100),
    IN iPreferredName VARCHAR(100),
    IN iPhoneNumber VARCHAR(20),
    IN iUserId INTEGER
)
BEGIN
	UPDATE client SET first_name = iFirstName, last_name = iLastName,
		preferred_name = iPreferredName, phone_number = iPhoneNumber,
        last_edited = CURRENT_TIMESTAMP(), user_id = iUserId
		WHERE id = iClientId;
END;
###END###;