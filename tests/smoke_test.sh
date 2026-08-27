#!/usr/bin/env bash
set -u

cd "$(dirname "$0")/.."
make clean >/dev/null
make >/dev/null

server_log="$(mktemp)"
client_log="$(mktemp)"
cleanup() {
    if [[ -n "${server_pid:-}" ]]; then
        kill "$server_pid" 2>/dev/null || true
    fi
    rm -f "$server_log" "$client_log"
}
trap cleanup EXIT

./server >"$server_log" 2>&1 &
server_pid=$!
sleep 0.3

printf 'pwd\nuname\nquit\n' | ./client >"$client_log"

grep -q 'StockFlow Security Lab' "$client_log"
grep -q '/home/ubuntu/lab-sockets-c' "$client_log"
grep -q '^> Linux' "$client_log"

echo 'Smoke test aprovado.'
