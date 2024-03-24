#include <mode/piano.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;


void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void putcharTUI(unsigned char, unsigned char, unsigned char, uint8_t, uint8_t);
void printfTUI(char*, uint8_t, uint8_t, uint8_t, uint8_t);

void sleep(uint32_t);
void makeBeep(uint32_t);


void pianoTUI() {
	
	TUI(0x09, 0x0c, 20, 8, 60, 16, true);
	printfTUI("Piano mode, press ctrl+c to exit.", 0x0f, 0x0c, 0, 0);
	printfTUI("Use the top 2 rows of keys ('1' - '=') and ('q' - ']')", 0x0f, 0x0c, 0, 1);
	printfTUI("to play notes. (1 = C, 2 = C# etc.) Hold shift/caps to shift down 2 octaves.", 0x0f, 0x0c, 0, 2);
	printfTUI("osakaOS piano program.", 0x0f, 0x09, 21, 9);
	printfTUI("Last note played: ", 0x0f, 0x09, 21, 11);
}


void notePlay(char ch, uint8_t octave, uint16_t time) {

	makeBeep(noteFreqVal(ch, octave));
	sleep(time);
}

//use for music things outside of piano mode :)
uint16_t noteFreqVal(char ch, uint8_t octave) {

	uint16_t noteFreq = 0;
	
	switch (octave) {
		
		case 3:
			switch (ch) {
			case 'C': noteFreq = 131; break;
			case 'c': noteFreq = 139; break;
			case 'D': noteFreq = 147; break;
			case 'd': noteFreq = 156; break;
			case 'E': noteFreq = 165; break;
			case 'F': noteFreq = 175; break;
			case 'f': noteFreq = 185; break;
			case 'G': noteFreq = 196; break;
			case 'g': noteFreq = 208; break;
			case 'A': noteFreq = 220; break;
			case 'a': noteFreq = 233; break;
			case 'B': noteFreq = 247; break;
			default: break;
			}
			break;
		case 4:
			switch (ch) {
			case 'C': noteFreq = 262; break;
			case 'c': noteFreq = 277; break;
			case 'D': noteFreq = 294; break;
			case 'd': noteFreq = 311; break;
			case 'E': noteFreq = 330; break;
			case 'F': noteFreq = 349; break;
			case 'f': noteFreq = 370; break;
			case 'G': noteFreq = 392; break;
			case 'g': noteFreq = 415; break;
			case 'A': noteFreq = 440; break;
			case 'a': noteFreq = 466; break;
			case 'B': noteFreq = 494; break;
			default: break;
			}
			break;
		case 5:
			switch (ch) {
			case 'C': noteFreq = 523; break;
			case 'c': noteFreq = 554; break;
			case 'D': noteFreq = 587; break;
			case 'd': noteFreq = 622; break;
			case 'E': noteFreq = 659; break;
			case 'F': noteFreq = 698; break;
			case 'f': noteFreq = 740; break;
			case 'G': noteFreq = 784; break;
			case 'g': noteFreq = 831; break;
			case 'A': noteFreq = 880; break;
			case 'a': noteFreq = 932; break;
			case 'B': noteFreq = 988; break;
			default: break;
			}
			break;
		case 6:
			switch (ch) {
			case 'C': noteFreq = 1047; break;
			case 'c': noteFreq = 1109; break;
			case 'D': noteFreq = 1175; break;
			case 'd': noteFreq = 1245; break;
			case 'E': noteFreq = 1319; break;
			case 'F': noteFreq = 1397; break;
			case 'f': noteFreq = 1480; break;
			case 'G': noteFreq = 1568; break;
			case 'g': noteFreq = 1661; break;
			case 'A': noteFreq = 1760; break;
			case 'a': noteFreq = 1865; break;
			case 'B': noteFreq = 1976; break;
			default: break;
			}
			break;
		default:
			break;
	}


	return noteFreq;
}

void piano(bool keypress, char key) {

        Speaker speaker;
        uint16_t noteFreq = 0;
	char* noteChar = "";

        if (keypress) {

                //key mappings to freq  
                switch (key) {

                        //speaker.NoSound();
			
			//lower octave #1

			//c
			case '!':
                                noteFreq = 131;
                                noteChar = "C-3 ";
				break;
			//c#
			case '@':
                                noteFreq = 139;
                                noteChar = "C#-3";
                                break;
			//d
			case '#':
                                noteFreq = 147;
                                noteChar = "D-3 ";
                                break;
			//d#
			case '$':
                                noteFreq = 156;
                                noteChar = "D#-3";
                                break;
			//e
			case '%':
                                noteFreq = 165;
                                noteChar = "E-3 ";
                                break;
			//f
			case '^':
                                noteFreq = 175;
                                noteChar = "F-3 ";
                                break;
			//f#
			case '&':
                                noteFreq = 185;
                                noteChar = "F#-3";
                                break;
			//g
			case '*':
                                noteFreq = 196;
                                noteChar = "G-3 ";
                                break;
			//g#
			case '(':
                                noteFreq = 208;
                                noteChar = "G#-3";
                                break;
			//a
			case ')':
                                noteFreq = 220;
                                noteChar = "A-3 ";
                                break;
			//a#
			case '_':
                                noteFreq = 233;
                                noteChar = "A#-3";
                                break;
			//b
			case '+':
                                noteFreq = 247;
                                noteChar = "B-3 ";
                                break;



			//lower octave #2

			//c
			case 'Q':
                                noteFreq = 262;
                                noteChar = "C-4 ";
                                break;
			//c#
			case 'W':
                                noteFreq = 277;
                                noteChar = "C#-4";
                                break;
			//d
			case 'E':
                                noteFreq = 294;
                                noteChar = "D-4 ";
                                break;
			//d#
			case 'R':
                                noteFreq = 311;
                                noteChar = "D#-4";
                                break;
			//e
			case 'T':
                                noteFreq = 330;
                                noteChar = "E-4 ";
                                break;
			//f
			case 'Y':
                                noteFreq = 349;
                                noteChar = "F-4 ";
                                break;
			//f#
			case 'U':
                                noteFreq = 370;
                                noteChar = "F#-4";
                                break;
			//g
			case 'I':
                                noteFreq = 392;
                                noteChar = "G-4 ";
                                break;
			//g#
			case 'O':
                                noteFreq = 415;
                                noteChar = "G#-4";
                                break;
			//a
			case 'P':
                                noteFreq = 440;
                                noteChar = "A-4 ";
                                break;
			//a#
			case '{':
                                noteFreq = 466;
                                noteChar = "A#-4";
                                break;
			//b
			case '}':
                                noteFreq = 494;
                                noteChar = "B-4 ";
                                break;



                        //upper octave #1
			
			//c
                        case '1':
                                noteFreq = 523;
                                noteChar = "C-5 ";
                                break;
                        //c#
                        case '2':
                                noteFreq = 554;
                                noteChar = "C#-5";
                                break;
                        //d
                        case '3':
                                noteFreq = 587;
                                noteChar = "D-5 ";
                                break;
                        //d#
                        case '4':
                                noteFreq = 622;
                                noteChar = "D#-5";
                                break;
                        //e
                        case '5':
                                noteFreq = 659;
                                noteChar = "E-5 ";
                                break;
                        //f
                        case '6':
                                noteFreq = 698;
                                noteChar = "F-5 ";
                                break;
                        //f#
                        case '7':
                                noteFreq = 740;
                                noteChar = "F#-5";
                                break;
                        //g
                        case '8':
                                noteFreq = 784;
                                noteChar = "G-5 ";
                                break;
                        //g#
                        case '9':
                                noteFreq = 831;
                                noteChar = "G#-5";
                                break;
                        //a
                        case '0':
                                noteFreq = 880;
                                noteChar = "A-5 ";
                                break;
                        //a#
                        case '-':
                                noteFreq = 932;
                                noteChar = "A#-5";
                                break;
                        //b
                        case '=':
                                noteFreq = 988;
                                noteChar = "B-5 ";
                                break;



                        //upper octave #2

                        //c
                        case 'q':
                                noteFreq = 1047;
                                noteChar = "C-6 ";
                                break;
                        //c#
                        case 'w':
                                noteFreq = 1109;
                                noteChar = "C#-6";
                                break;
                        //d
                        case 'e':
                                noteFreq = 1175;
                                noteChar = "D-6 ";
                                break;
                        //d#
			case 'r':
                                noteFreq = 1245;
                                noteChar = "D#-6";
                                break;
                        //e
                        case 't':
                                noteFreq = 1319;
                                noteChar = "E-6 ";
                                break;
                        //f
                        case 'y':
                                noteFreq = 1397;
                                noteChar = "F-6 ";
                                break;
                        //f#
                        case 'u':
                                noteFreq = 1480;
                                noteChar = "F#-6";
                                break;
                        //g
                        case 'i':
                                noteFreq = 1568;
                                noteChar = "G-6 ";
                                break;
                        //g#
                        case 'o':
                                noteFreq = 1661;
                                noteChar = "G#-6 ";
                                break;
                        //a
                        case 'p':
                                noteFreq = 1760;
                                noteChar = "A-6 ";
                                break;
                        //a#
                        case '[':
                                noteFreq = 1865;
                                noteChar = "A#-6";
                                break;
                        //b
                        case ']':
                                noteFreq = 1976;
                                noteChar = "B-6 ";
                                break;
                        default:
				return;
                                break;
                }
                printfTUI(noteChar, 0x0f, 0x09, 39, 11);
		speaker.PlaySound(noteFreq);
        }

        //stop sound when key not pressed
        if (!keypress) {

		sleep(1);
                speaker.NoSound();
        }
}



