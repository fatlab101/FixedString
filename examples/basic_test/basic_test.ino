// FixedString
// This sketch demonstrates how to use various features of FixedStrings
//
// This example code is in the public domain.
//

#include <FixedString.h>

static int fail_cnt=0;
void test(bool b_val,int test_no,const char* val="")
{
  FixedString<28> s;
 if(!b_val)
   {
    FixedString<8> s_val(val);
    if(!s_val.empty())
      s.format("Test No: %i - Failed <%s>",test_no,s_val.c_str());
    else
      s.format("Test No: %i - Failed....",test_no);
    Serial.println(s);
    fail_cnt++;
   }
}
void test_equals(const char* s1,const char* s2,int test_no)
{
  if(strcmp(s1,s2)!=0)
   {
   FixedString<64> sFmt(F("Test No: %i - Failed got: %s, expected: %s"));
   FixedString<92> s;
   s.format(sFmt,test_no,s1,s2);
   Serial.println(s);
   fail_cnt++;
   }
}
DEFINE_PSTR(cFlashAlphabet,"abcdefghijklmnopqrstuvwxyz");

void TestFixedString()
{
	FixedString<32> s2(GET_PSTR(cFlashAlphabet));
	test(s2.length()==26,1);
	test(s2.available()==4,2);
	test(s2.charAt(3)=='d',3);
	test(s2.charAt(0)=='a',4);
	test(s2[25]=='z',5);
	s2.setCharAt(3,'D');
	test(s2.charAt(3)=='D',6);

	FixedString<32> s("fred ");
	s.concat("fish");
	s+="_23";
	test(s=="fred fish_23",10);
	test(s.substring(5)=="fish_23",11);
	s.remove(2,2);
	test(s=="fr fish_23",15);
	s.insert(2,"ed");
	test(s=="fred fish_23",16);
	test(s.length()==12,17);
	s+=GET_PSTR(cFlashAlphabet);
	test(s.length()==30,18);
	test(s.full(),19);

	FixedString<32> s1;
	s1+="abc";
	s1+="def";
	s1+="ghij";
	s1+="klm";
	s1+="nopq";
	s1+="rst";
	s1+="uvw";
	s1+="xyz";
	test(s1==GET_PSTR(cFlashAlphabet),20);
	test(s1.length()==26,21);
	s1.replace("def","DEF");
	test(s1!=s2,22);
	test(s1.equalsIgnoreCase(GET_PSTR(cFlashAlphabet)),23);

	test(FixedString<8>(3)=="3",30);
	test(FixedString<8>(3l)=="3",31);
	test_equals(FixedString<8>(3.348,2),"3.35",32);
	test_equals(FixedString<8>(3.342,2),"3.34",33);
	test(FixedString<8>(33u,base10)=="33",34);
	test(FixedString<8>(33u,base8)=="41",35);
	test(FixedString<8>(33u,base16)=="21",36);

	FixedString<16> s8;
	s8.assign(255u,base16);
	test(s8=="ff",37);


	FixedString<16> s16("buddy boy 21");
	s1=s16;
	s1.remove(9);
	test(s1=="buddy boy",40);
	s1.toUpperCase();
	test(s1=="BUDDY BOY",41);
	s1.toLowerCase();
	test(s1=="buddy boy",42);


	test(s1.indexOf('u')==1,50);
	test(s1.indexOf('y')==4,51);
	test(s1.indexOf('y',5)==8,52);
	test(s1.indexOf('u')==1,53);
	test(s1.indexOf("ud")==1,54);

	test(s1.startsWith("bud"),60);
	test(s1.endsWith("boy"),61);

	test(s1.substring(2,4)=="dd",62);

	s1=s16;
	s1.remove(9);
	test(s1=="buddy boy",63);
	s1.remove(4,2);
	test(s1=="buddboy",64);
	s1.insert(4,"__");
	test(s1=="budd__boy",65);
	s1.insert(4,"abc",2);
	test(s1=="buddab__boy",66);
	s1.insert(4,4,'z');
	test(s1=="buddzzzzab__boy",67);
	s1.replace('z','a');
	test(s1=="buddaaaaab__boy",68);
	s1.replace("aaa","jjj");
	test(s1=="buddjjjaab__boy",69);
	s1.replace("jjj","");
	test(s1=="buddaab__boy",70);

	s1="	 fred is a knob.   ";
	s1.trim();
	test(s1=="fred is a knob.",80);
	s1="      ";
	s1.trim();
	test(s1.empty(),62);

	s1="wha: ";
	s1+=3;
	s1+=3l;
	s1+=3u;
	s1+='c';
	s1+="de";
	s1+=4.349;
	test_equals(s1,"wha: 333cde4.35",81);
	s16=s1;
	test_equals(s16,"wha: 333cde4.3",82);

	//Format test
	s1.format("%s %i %u ","fred",-23,341u,23.34);
	test_equals(s1,"fred -23 341 ",90);

	FixedString<24> s24 = "test me:"+s1;
	test_equals(s24,"test me:fred -23 341 ",91);
	s24 = 3.458 + s1;
	test_equals(s24,"3.46fred -23 341 ",92);
	s24 = -3456 + s1;
	test_equals(s24,"-3456fred -23 341 ",93);
	s24 = 3456u + s1;
	test_equals(s24,"3456fred -23 341 ",94);

	FixedString<32> sp(F("This is a program string"));
	test_equals(sp,"This is a program string",100);
	sp+=F(" ++");
	test_equals(sp,"This is a program string ++",101);
	sp=F("P string2 ") + s24;
	//NB. Will overflow on str<24> so test len == 22!!!
	test(sp="P string2 3456fred -23",102);


	FixedString<64> s25(25.456,2);
	test_equals(s25, "25.46", 103);

	s=25;
	test_equals(s25, "25", 104);
	test(s25.toInt()== 25, 105);
	s25 += " stuff at u end [tu]*e#$d";

	test(s25.indexOf('u') == 5, 106);
	test(s25.indexOf('u', 6) == 12, 107);
	test(s25.indexOf('u', 13) == 20, 108);

	test(s25.lastIndexOf('u') == 20, 109);
	test(s25.lastIndexOf('u', 10) == 5, 110);

	test(s25.indexOf("tu") == 4, 111);
	test(s25.indexOf("tu", 6) == 19, 112);

	test(s25.lastIndexOf("tu") == 19, 113);
	test(s25.lastIndexOf("tu", 18) == 4, 114);
}

void setup() 
{
    Serial.begin(9600);
    delay(50);
    Serial.println("Basic testing of FixedString<>....");
	TestFixedString();
    Serial.println("Finished testing");
    if(fail_cnt>0)
      return;
    Serial.println("All tests passed");   
}

void loop(){}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
