program extra; 


var i: integer; 
const A = 10; 
var k: array [-5 .. 15] of integer; 

/*
extern procedure some();
extern keyword must preced function/procedure keyword 
*/

procedure passing(i: integer*);
begin
    readln(i);
end; 


/* attributes proceed functions declarations, togather with the extern keyword if used */
@NoReturn
@Inline 
procedure willNotReturn();
begin 
    _exit(1);
end; 

function getValue(): integer;
begin 
    getValue := -5; 
end;

function guess_number(): integer; 
var i: integer;
begin 
    echo("Please give number");
    passing(i&);
    print("Your number is: ");
    writeln(i);
    switch i:
    case A then begin /* only constants in switch, that includes global constants, not only number literals */ 
        guess_number := 1; 
    end;
    case 15 then 
        guess_number := 1 
    ; 
    default begin 
        guess_number := 0;
    end; 
end; 

begin 
    i := 10; 
    k[getValue()] := 1;

    k[-2] := 0xAFC;
    k[-3] := 0065;

    if k[-2] > 0 and k[-3] <> 0 then 
        echo("Diff")
    ; 

    writeln(k[-2]);
    writeln(k[-3]);

    write(5);
    print(' ');
    writeln(10);
    if guess_number() = 1 then 
        echo('I guessed it!')
    else 
        echo("I didn't guess"); 
    
    for i := 1 to 4 do begin 
        write(i);
        i := 3; 
    end;
    newLine();
    writeln(i); /* note local scope of variables - i is a global variable, yet used in the for loop and even assigned there 
        this expression will still result in assigned value 3 being printed 
        theres no need for defining global variables for loops, they work only within scope of the loop and are immutable
    */ 


    for i := 1 until 4 do /* single expression - no need for block ... end; */ 
        write(i)
    ; /* single statements block must not be empty and finish with semicolon */ 

    newLine();

    for i := 2 to 8 do 
        if i mod 2 = 0 then
            write(i)
        ;
    ; /* in case of single statement expression like assignment, exit keyword or function call semicolon can be omitted 
         but as there's no else keyword, `if` needs it semicolon despite being single expression block as it's an advanced statement
         and then there's another semicolon to indicate end of single expression `for` loop block 
        */ 
    newLine();

    for i := 6 downto 1 do
        switch i:
        case 1 then begin 
            echo("Finishing loop");
        end;
        case 6 downuntil 2 then begin
            write(i * 2);
            print(' '); 
        end; 
        default 
            write(i)
        ;
    ;

    for i := 4 downuntil 1 do begin 
        write(i);
    end; 
    
    newLine();
    
end. 