REM source: http://www.lemon64.com/manual/
NEW
10 POKE 53281,1: PRINT"*"
20 POKE 53280,7: POKE 53281,6
30 X = 1 : Y = 1
40 DX = 1 : DY = 1
50 POKE 1024 + X + 40 * Y, 81
60 FOR T = 1 TO 10 : NEXT
70 POKE 1024 + X + 40 * Y, 32
80 X = X + DX
90 IF X <= 0 OR X >= 39 THEN DX = -DX
100 Y = Y + DY
110 IF Y <= 0 OR Y >= 24 THEN DY = -DY
120 GOTO 50
