/************************
	DATABASE FOR SA BOT
*************************/


#Tables keeps usernames
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS sabot.`user`;
CREATE TABLE sabot.`user` (
	`id` INT UNSIGNED AUTO_INCREMENT NOT NULL,
	`name` VARCHAR(32) NOT NULL,
	`login` INT UNSIGNED,
	`password` VARCHAR(20), 	#This is being unrealistically optimistic.
	`ip` INT UNSIGNED,		#This is being optimistic. 
	`is_mod` BIT(1) NOT NULL,
	`misc` BLOB, 
	PRIMARY KEY(`id`), 
 	CONSTRAINT FOREIGN KEY(`login`) REFERENCES sabot.`login`(`id`)	
);

DROP TABLE IF EXISTS sabot.`performance`;
CREATE TABLE sabot.`performance`(
	`user` INT UNSIGNED, 
	`timeshot` TIMESTAMP,
	`kd_ratio` FLOAT(23),
	`kills` INT UNSIGNED,
	`rounds_cmpl` INT UNSIGNED,
	`rounds_cmplp` FLOAT(23),
	PRIMARY KEY(`user`,`timeshot`),
	CONSTRAINT FOREIGN KEY(`user`) REFERENCES sabot.`user`(`id`)
);

DROP TABLE IF EXISTS sabot.`login`;
CREATE TABLE sabot.`login`(
	`id` INT UNSIGNED AUTO_INCREMENT NOT NULL,
	`handle` CHAR(3),
	`server` INT UNSIGNED,
	`logged_in` TIMESTAMP,
	`logged_out` TIMESTAMP,
	`time_known` BIT(1),
	PRIMARY KEY(`id`),
	CONSTRAINT FOREIGN KEY(`server`) REFERENCES sabot.`server`(`id`)
);

DROP TABLE IF EXISTS sabot.`game`;
CREATE TABLE sabot.`game`(
	`id` INT UNSIGNED AUTO_INCREMENT NOT NULL,
	`server` INT UNSIGNED,
	`arena` VARCHAR(20),
	`name` VARCHAR(20),
	`start` TIMESTAMP,
	`end` TIMESTAMP,
	`time_known` BIT(1),
	PRIMARY KEY(`id`),
	CONSTRAINT FOREIGN KEY(`server`) REFERENCES sabot.`server`(`id`)
);

DROP TABLE IF EXISTS sabot.`server`;
CREATE TABLE sabot.`server`(
	`id` INT UNSIGNED AUTO_INCREMENT NOT NULL,
	`name` VARCHAR(32),
	`ip` INT UNSIGNED,
	PRIMARY KEY(`id`)
);

DROP TABLE IF EXISTS sabot.`message`;
CREATE TABLE sabot.`message`(
	`id` INT UNSIGNED AUTO_INCREMENT NOT NULL, 
	`message` VARCHAR(50),
 	`message_to` INT UNSIGNED, 	#being optimistic if we can get pm's, otherwise works for my own pm's
	`message_from` INT UNSIGNED NOT NULL,
	`login` INT UNSIGNED NOT NULL,
	`flag` TINYINT UNSIGNED DEFAULT 0,
	PRIMARY KEY(`id`),
	CONSTRAINT FOREIGN KEY(`message_to`) REFERENCES sabot.`user`(`id`),
	CONSTRAINT FOREIGN KEY(`message_from`) REFERENCES sabot.`user`(`id`),
	CONSTRAINT FOREIGN KEY(`login`) REFERENCES sabot.`login`(`id`)
);

#using sqlite syntax

CREATE TABLE account(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	user_name VARCHAR(20),
	password VARCHAR(20)
);

SET FOREIGN_KEY_CHECKS = 1;

INSERT INTO sabot.server(name, ip) VALUES('2-Dimensional Central', INET_ATON('74.86.43.9'));
INSERT INTO sabot.server(name, ip) VALUES('Paper Thin City (RP)', INET_ATON('74.86.43.8'));
INSERT INTO sabot.server(name, ip) VALUES('Fine Line Island', INET_ATON('67.19.138.234'));
INSERT INTO sabot.server(name, ip) VALUES('U of SA', INET_ATON('67.19.138.236'));
INSERT INTO sabot.server(name, ip) VALUES('Flat World', INET_ATON('74.86.3.220'));
INSERT INTO sabot.server(name, ip) VALUES('Planar Outpost', INET_ATON('67.19.138.235'));
INSERT INTO sabot.server(name, ip) VALUES('Mobius Metropolis', INET_ATON('74.86.3.221'));
INSERT INTO sabot.server(name, ip) VALUES('{Europe} Amsterdam', INET_ATON('94.75.214.10'));
INSERT INTO sabot.server(name, ip) VALUES('{Compatibility} Sticktopia', INET_ATON('74.86.3.222'));
INSERT INTO sabot.server(name, ip) VALUES('{Lab Pass} The SS Lineage', INET_ATON('0.0.0.0'));	#unknown, so use a 'null' address


