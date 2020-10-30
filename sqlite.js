const fs = require('fs');
const child_process = require('child_process');

module.exports = { connect };

function connect(sqlitePath, dbPath) {
    let sqlite = null;
    let requestBuffer = []
    let responseBuffer = [];

    let init = () => {
        sqlite = child_process.spawn(sqlitePath, [dbPath]);

        sqlite.stdout.on('data', data =>
            processResponse(sqlite, requestBuffer, responseBuffer, data.toString()));

        sqlite.on('close', () => {
            if (requestBuffer.length > 0) {
                let {onError, onSuccess} = requestBuffer.shift();
                onError('Connection closed');
            }

            init();
        });

        if (requestBuffer.length > 0)
            sendQuery(sqlite, requestBuffer, responseBuffer);
    };

    init();

    return {
        expr: (sql, ...params) =>  {
            return [sql, ...params]
                .map(x => x.toString().replace(/\n/g, ''))
                .join('\n');
        },

        query: (sql, onError, onSuccess) => {
            let qCount = sql.split('').reduce((acc, x) => acc += (x === '?' ) ? 1 : 0, 0);
            let lCount = sql.split('').reduce((acc, x) => acc += (x === '\n') ? 1 : 0, 0);

            if (qCount != lCount)
                return onError && onError('Malformed expression; mismatched parameters');

            requestBuffer.push({
                sql,
                onError: onError || (err => console.log(err)),
                onSuccess: onSuccess || (data => {}),
            });

            if (requestBuffer.length == 1)
                sendQuery(sqlite, requestBuffer, responseBuffer);
        },
    };
}

function sendQuery(sqlite, requestBuffer, responseBuffer) {
    if (requestBuffer.length == 0)
        return;

    responseBuffer.length = 0;

    let {sql, onError, onSuccess} = requestBuffer[0];

    sqlite.stdin.write(`${sql}\n`);
}

function processResponse(sqlite, requestBuffer, responseBuffer, str) {
    if (requestBuffer.length == 0)
        return;

    responseBuffer.push(str);
    
    let lastStr = responseBuffer[responseBuffer.length - 1];

    if (lastStr.slice(-6) === 'QUERY\n') {
        // End of response

        responseBuffer = responseBuffer
            .join('')
            .split('\n')
            .filter(x => x);

        let rows = [];
        let currentRow = {};
        for (let i = 0; i < responseBuffer.length; i++) {
            let line = responseBuffer[i];

            if (line === 'QUERY') {
                break;
            } else if (line === 'ROW') {
                rows.push(currentRow);
                currentRow = {};
            } else {
                let nextLine = responseBuffer[i + 1];
                currentRow[line] = (nextLine == '(null)') ? null : nextLine;
                i++;

                if (line === 'FAIL')
                    break;
            }
        }

        let {onError, onSuccess} = requestBuffer[0];

        if (currentRow['FAIL']) {
            onError(currentRow['FAIL']);
        } else {
            onSuccess(rows);
        }

        // This has to be done directly before in case something gets added after the callback
        // There was a nasty race condition where `sendQuery` would be called twice per query,
        // once here and another time when the query was added because of the unsynchronized request buffer
        requestBuffer.shift();
        if (requestBuffer.length > 0)
            sendQuery(sqlite, requestBuffer, responseBuffer);
    }
}
