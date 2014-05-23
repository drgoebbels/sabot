ATTACH DATABASE 'sabot.db' AS 'sabot';

DROP TABLE IF EXISTS sabot.user;
CREATE TABLE sabot.user(
  id INTEGER NOT NULL, 
  name VARCHAR(32) UNIQUE NOT NULL,
  password VARCHAR(20),
  PRIMARY KEY(id)
); 

DROP TABLE IF EXISTS sabot.login;
CREATE TABLE sabot.login(
  id INTEGER NOT NULL, 
  user INTEGER NOT NULL,
  handle CHAR(3),
  server INTEGER,
  enter TIMESTAMP,
  exit TIMESTAMP,
  time_bounds TINYINT, /* would have preferred BIT(2)*/
  kd_ratio REAL, 
  kills INTEGER,
  rounds_comp INTEGER,
  rounds_compp REAL,
  color INTEGER,
  FOREIGN KEY(server) REFERENCES server(id),
  FOREIGN KEY(user) REFERENCES user(id),
  PRIMARY KEY(id)
);

DROP TABLE IF EXISTS sabot.game;
CREATE TABLE sabot.game(
  id INTEGER NOT NULL, 
  login INTEGER NOT NULL, 
  arena VARCHAR(20),
  name VARCHAR(32), 
  start TIMESTAMP,
  end TIMESTAMP, 
  time_bounds TINYINT, 
  FOREIGN KEY(login) REFERENCES login(id), 
  PRIMARY KEY(id) 
);

DROP TABLE IF EXISTS sabot.server;
CREATE TABLE sabot.server(
  id INTEGER NOT NULL, 
  ip VARCHAR(16) NOT NULL, 
  name VARCHAR(32),
  PRIMARY KEY(id)
);

DROP TABLE IF EXISTS sabot.message; 
CREATE TABLE sabot.message(
  id INTEGER NOT NULL, 
  message TEXT, 
  type TINYINT,
  sender INTEGER, 
  FOREIGN KEY(sender) REFERENCES login(id),
  PRIMARY KEY(id)
);

DROP TABLE IF EXISTS aes_key;
CREATE TABLE aes_key(
  id INTEGER NOT NULL, 
  name VARCHAR(32),  
  key BLOB, 
  PRIMARY KEY(id)
);

/* Keys that are indexed */
CREATE INDEX username_index ON user(name);
CREATE INDEX userpkey_index ON user(id);

INSERT INTO sabot.server(ip, name) VALUES("74.86.43.9", "2 Dimensional Central");
INSERT INTO sabot.server(ip, name) VALUES("74.86.43.8", "Paper Thin City");
INSERT INTO sabot.server(ip, name) VALUES("67.19.138.234", "Fine Line");
INSERT INTO sabot.server(ip, name) VALUES("67.19.138.236", "U Of SA");
INSERT INTO sabot.server(ip, name) VALUES("74.86.3.220", "Flat World");
INSERT INTO sabot.server(ip, name) VALUES("67.19.138.235", "Planar Outpost");
INSERT INTO sabot.server(ip, name) VALUES("74.86.3.221", "Mobius Metropolis");
INSERT INTO sabot.server(ip, name) VALUES("94.75.214.10", "EU Amsterdam");
INSERT INTO sabot.server(ip, name) VALUES("74.86.3.222", "Compatibility (Sticktopia)");
INSERT INTO sabot.server(ip, name) VALUES("74.86.43.10", "SS Lineage");




