# keytop machine protocol

`keytop value` keeps the former `key sysmon` JSON contract at schema version 1. Numeric
values keep their existing units: bytes are bytes, rates are bytes per second, temperatures
are Celsius, frequencies are MHz, and percentages are `0..100`.

```bash
keytop value snapshot --format json
keytop value stream --format jsonl --interval 1000 --modules system,cpu,memory,network
keytop value processes --format json --limit 20 --sort cpu
```

`stream` emits one complete JSON object per line, flushes after each line, and stops on
SIGINT/SIGTERM/SIGHUP. `processes` is collected only when requested. Unavailable numeric
values are JSON `null`; partial sensors are represented in the `errors` array rather than
being replaced with fake zeros.

