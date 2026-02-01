; Nested include with conditionals like biosdpb.mac
  IF ((diskdi mod 10000)/1000) eq 4
	LD B,10
   IF (diskdi mod 100) eq 1
	LD C,20
   ELSE
	LD C,30
   ENDIF
  ENDIF
