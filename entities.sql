
SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

# Datenbank: 'volkszaehler'


CREATE TABLE IF NOT EXISTS entities 
(
  id         int(11)      NOT NULL AUTO_INCREMENT,
  uuid       varchar(36)  NOT NULL,
  type       varchar(255) NOT NULL,
  class      varchar(255) NOT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY entities_uuid_uniq (uuid)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS data
(
  id         int(11)     NOT NULL AUTO_INCREMENT, 
  channel_id int(11)     NOT NULL,
  timestamp  bigint(20)  NOT NULL,
  value      double      NOT NULL,
  PRIMARY KEY (id),
  FOREIGN KEY (channel_id) REFERENCES entities (id),
  UNIQUE KEY unique_properties (channel_id, timestamp)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

# -- --------------------------------------------------------

CREATE TABLE IF NOT EXISTS properties
(
  id         int(11)      NOT NULL AUTO_INCREMENT,
  entity_id  int(11)      DEFAULT NULL,
  pkey       varchar(255) NOT NULL,
  value      longtext     NOT NULL,
  PRIMARY KEY (id),
  FOREIGN KEY (entity_id) REFERENCES entities (id),
  UNIQUE KEY unique_properties (id, pkey),
  KEY entity_id (entity_id)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=15 ;

CREATE TABLE IF NOT EXISTS aggregate 
(
  id         int(11)     NOT NULL AUTO_INCREMENT, 
  channel_id int(11)     DEFAULT NULL,
  type       smallint(6) NOT NULL, 
  timestamp  bigint(20)  NOT NULL,
  value      double      NOT NULL,
  count      int(11)     NOT NULL,
  PRIMARY KEY (id),
  FOREIGN KEY (channel_id) REFERENCES entities (id),
  UNIQUE KEY unique_properties (channel_id, timestamp)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=15 ;

CREATE TABLE IF NOT EXISTS entities_in_aggregator
(
  parent_id  int(11)     NOT NULL,
  child_id   int(11)     NOT NULL,
  PRIMARY KEY (parent_id, child_id),
  FOREIGN KEY (parent_id) REFERENCES entities (id),
  FOREIGN KEY (child_id) REFERENCES entities (id),
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

# -- --------------------------------------------------------


INSERT entities(uuid, type, class) VALUES("26da8400-6278-11e3-bb7d-cfe455640e79",
  "Rücklaufist.", "class");

INSERT entities(uuid, type, class) VALUES("2a93db10-6278-11e3-a7ba-c580609d3f60",
  "Ventil", "class");

INSERT entities(uuid, type, class) VALUES("62d2be10-6278-11e3-80c4-0df91549a95f",
  "Aussentemp.", "class");

INSERT entities(uuid, type, class) VALUES("62d2be10-6278-11e3-80c4-1df91549a95f",
  "Vorlaufsoll.", "class");

INSERT entities(uuid, type, class) VALUES("62d2be10-6278-11e3-80c4-2df91549a95f",
  "Vorlaufist.", "class");

INSERT entities(uuid, type, class) VALUES("62d2be10-6278-11e3-80c4-3df91549a95f",
  "Speichersoll.", "class");

INSERT entities(uuid, type, class) VALUES("62d2be10-6278-11e3-80c4-4df91549a95f",
  "Speicherist.", "class");


