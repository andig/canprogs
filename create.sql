
DROP TABLE can_log;

CREATE TABLE can_log
(
  id          INT     UNSIGNED NOT NULL AUTO_INCREMENT,
  counter     INT     UNSIGNED NOT NULL,
  timestamp   BIGINT  NOT NULL,
  can_id      INT     UNSIGNED NOT NULL,
  d0          TINYINT UNSIGNED NULL,
  d1          TINYINT UNSIGNED NULL,
  d2          TINYINT UNSIGNED NULL,
  d3          TINYINT UNSIGNED NULL,
  d4          TINYINT UNSIGNED NULL,
  d5          TINYINT UNSIGNED NULL,
  d6          TINYINT UNSIGNED NULL,
  d7          TINYINT UNSIGNED NULL,
  PRIMARY KEY (id),
  UNIQUE KEY unique_properties (timestamp, can_id)
);
