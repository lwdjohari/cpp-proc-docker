
-- create SCOTT user with password TIGER
CREATE USER scott IDENTIFIED BY tiger
  DEFAULT TABLESPACE users
  TEMPORARY TABLESPACE temp
  QUOTA UNLIMITED ON users;

-- grant basic privileges
GRANT CONNECT, RESOURCE TO scott;

-- switch into SCOTT
ALTER SESSION SET CURRENT_SCHEMA = scott;

-- create EMPLOYEES table (simplified for Pro*C demo)
CREATE TABLE employees (
    id      NUMBER(6)    PRIMARY KEY,
    name    VARCHAR2(50) NOT NULL,
    dept    VARCHAR2(30),
    sal     NUMBER(8,2)
);

-- insert a few sample rows
INSERT INTO employees (id, name, dept, sal) VALUES (101, 'Scott',   'SALES', 2000);
INSERT INTO employees (id, name, dept, sal) VALUES (102, 'Tiger',   'HR',    2500);
INSERT INTO employees (id, name, dept, sal) VALUES (103, 'Adams',   'IT',    3000);
INSERT INTO employees (id, name, dept, sal) VALUES (104, 'Johnson', 'SALES', 2800);

COMMIT;
