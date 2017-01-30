program expressions;

const
    Multiplyer = 5;
var
    n: integer;

begin
    readln(n&); /* ptr */
    n := (n - 1) * Multiplyer + 10;
    writeln(n);
end.