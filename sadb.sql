ATTACH DATABASE 'sabot.db' AS 'sabot';

DROP TABLE IF EXISTS sabot.user;
CREATE TABLE sabot.user(
  id INTEGER NOT NULL, 
  name VARCHAR(32) NOT NULL,
  password VARCHAR(20),
  PRIMARY KEY(id)
);

DROP TABLE IF EXISTS sabot.performance;
CREATE TABLE sabot.performance(
  id INTEGER NOT NULL, 
  login INTEGER NOT NULL, 
  kd_ratio REAL, 
  kills INTEGER,
  rounds_comp INTEGER,
  rounds_compp REAL,
  FOREIGN KEY(login) REFERENCES login(id)
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
  message_to INTEGER, 
  message_from INTEGER, 
  flag TINYINT DEFAULT 0, 
  FOREIGN KEY(message_to) REFERENCES user(id),
  FOREIGN KEY(message_from) REFERENCES user(id), 
  PRIMARY KEY(id)
);

DROP TABLE IF EXISTS aes_key;
CREATE TABLE aes_key(
  id INTEGER NOT NULL, 
  name VARCHAR(32),  
  key BLOB, 
  PRIMARY KEY(id)
);


