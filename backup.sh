#!/bin/sh

sqlite3 db.sqlite ".backup '_backup_db.sqlite'"
