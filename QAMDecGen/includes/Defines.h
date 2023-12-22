/*
 * Defines.h
 *
 * Created: 22.12.2023 08:08:34
 *  Author: mikaj
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_

#define BitMask 0x000000FF
#define Idel0 0
#define Idel1 1
#define type 2
#define sync 3
#define Data 4
#define checksum 5
#define FINAL 6


#define quarterjump1 7
#define quarterjump2 8 //Perfekt f?r Sync Weil diese Spr?nge nur in einem Fall auftreten k?nnen 3 -> 0
#define quarterjump3 9
#define quarterjump4 10
#define quarterjump5 11
#define quarterjump6 12

#define halfjump0 14
#define halfjump1 15
#define halfjump2 16
#define halfjump3 17
#define halfjump4 18
#define halfjump5 19
#define halfjump6 20
#define halfjump7 21

#define threequartersjump0 22
#define threequartersjump1 23
#define threequartersjump2 24
#define threequartersjump3 25
#define threequartersjump4 26
#define threequartersjump5 27
#define threequartersjump6 28

#define fulljump0 30
#define fulljump1 31
#define fulljump2 32
#define fulljump3 33
#define fulljump4 34
#define fulljump5 35
#define fulljump6 36


#define onequarterjump5 37
#define onequarterjump0 38
#define onequarterjump1 39
#define onequarterjump2 40
#define onequarterjump3 41
#define onequarterjump4 42
#define onequarterjump6 43
#define onequarterjump7 44

#define onehalfjump5 45
#define onehalfjump0 46
#define onehalfjump1 47
#define onehalfjump2 48
#define onehalfjump3 49
#define onehalfjump4 50
#define onehalfjump6 51
#define onehalfjump7 52

#define onethreequartersjump5 53
#define onethreequartersjump0 54
#define onethreequartersjump1 55
#define onethreequartersjump2 56 //Perfekt f?r Sync Weil diese Spr?nge nur in einem Fall auftreten k?nnen 0 -> 3
#define onethreequartersjump3 57
#define onethreequartersjump4 58
#define onethreequartersjump6 59


#define HI 2150
#define LO 1200



#endif /* DEFINES_H_ */