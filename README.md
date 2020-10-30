# Storage
### Self-contained sqlite3 setup for JavaScript

## Dependencies
* gcc
* node.js
* sqlite3 (for backups only)

## How to use
Run `make` to build `sqlite` binary.
`sqlite` binary must be run with the db filename as the argument (non-existant name is okay).
When running, put enter query on one line. Extra parameters can be specified with a `?`, and each parameter goes on a separate line after the query line.

To use the `sqlite.js` module, write:
```
let sqlite = require('./sqlite.js');
let db = sqlite.connect('./sqlite', './db.sqlite');

db.query(db.expr('INSERT INTO table1 (column1, column2) VALUES (1, ?)', 'increscent'),
    err => console.log(err),
    data => console.log(data)
);

db.query('SELECT * FROM table1',
    err => console.log(err),
    data => console.log(data)
);
```
