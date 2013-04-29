-- Copyright (c) 2010, 2013, Oracle and/or its affiliates. All rights reserved.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; version 2 of the License.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET storage_engine=NDB;
-- SET storage_engine=INNODB;

DROP DATABASE IF EXISTS crunddb;
CREATE DATABASE crunddb;
USE crunddb;

-- DROP TABLE IF EXISTS B;
-- DROP TABLE IF EXISTS A;

CREATE TABLE A (
        id              INT             NOT NULL,
        cint            INT,
        clong           BIGINT,
        cfloat          FLOAT,
        cdouble         DOUBLE,
        CONSTRAINT PK_A_0 PRIMARY KEY (id)
);

CREATE TABLE B (
        id              INT             NOT NULL,
        cint            INT,
        clong           BIGINT,
        cfloat          FLOAT,
        cdouble         DOUBLE,
        a_id            INT,
 	-- XXX crund c++ code currently does not support VARBINARY/CHAR > 255
	cvarbinary_def  VARBINARY(202),
        cvarchar_def    VARCHAR(202),
        cblob_def       BLOB(1000004),
        ctext_def       TEXT(1000004),
        CONSTRAINT PK_B_0 PRIMARY KEY (id),
        CONSTRAINT FK_B_1 FOREIGN KEY (a_id) REFERENCES a (id)
);
--        cvarchar_ascii  VARCHAR(202) CHARACTER SET ASCII,
--        ctext_ascii     TEXT(202) CHARACTER SET ASCII,
--        cvarchar_ucs2   VARCHAR(202) CHARACTER SET UCS2,
--        ctext_ucs2      TEXT(202) CHARACTER SET UCS2,
--        cvarchar_utf8   VARCHAR(202) CHARACTER SET UTF8,
--        ctext_utf8      TEXT(202) CHARACTER SET UTF8,

CREATE INDEX I_B_FK ON B (
        a_id
);
