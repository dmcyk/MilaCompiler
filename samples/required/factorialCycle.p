program factorialCycle;

var
    n: integer;
    f: integer;
begin
    f := 1;
    readln(n&);  /* ptr */
    while(n >= 2) do begin
        f := f * n;
	    dec(n&); /* ptr */ 
    end;
    writeln(f);
end.
