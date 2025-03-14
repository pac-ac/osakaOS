#include <art.h>

using namespace os;
using namespace os::common;
using namespace os::math;

void printf(char* str);
void putcharTUI(unsigned char, unsigned char, unsigned char, uint8_t, uint8_t);
void printfLine(const char* str, uint8_t line);


void os::cubeScreen(int backgroundASCIICode, float incrementSpeed, Cube* data) {


	//memset(data->buffer, backgroundASCIICode, data.width*data.height);
	for (int i = 0; i < data->width*data->height; i++) {
		data->buffer[i] = backgroundASCIICode;
	}
		
	//memset(data->zBuffer, 0, data.width*data.height*4);
	for (int i = 0; i < data->width*data->height*4; i++) {
		data->zBuffer[i] = 0;
	}
		
	data->cubeWidth = 20;
	data->horizontalOffset = -2 * data->cubeWidth;

	//first cube
	calculateCube(incrementSpeed, data);

	data->cubeWidth = 10;
	data->horizontalOffset = 1 * data->cubeWidth;
		
	//second cube
	calculateCube(incrementSpeed, data);
		
		
	data->cubeWidth = 5;
	data->horizontalOffset = 8 * data->cubeWidth;
		
	//third cube
	calculateCube(incrementSpeed, data);

	//clear screen from previous frame
	printf("\v");

	for (int k = 0; k < data->width*data->height; k++) {
	
		char* str = " ";
		str[0] = (k % data->width ? data->buffer[k] : 10);
		printf(str);
	}

	data->A += 0.5;
	data->B += 0.5;
	data->C += 0.1;
}


void os::osakaAscii() {
		
       	printfLine("                  __   ", 12);
        printfLine("     ######      /  \\ ", 13);
	printfLine("   #,,,,,v!5#       |  ", 14);
	printfLine("  #,,,,,,,v!5#     /   ", 15);
	printfLine("  #,,,,,,,v!5#     |   ", 16);
	printfLine("  #,,,,,,,v!5#     .   ", 17);
	printfLine("  #,,,,,,vv!5#                                     PRESS ANY KEY TO", 18);
	printfLine("  v#,,,,vv!5#v                                    START COMMAND LINE", 19);
	printfLine("   #,,,,vv5W#                                            ^_^", 20);
	printfLine("  /|v#,v!5#v|\\        ", 21);
	printfLine("  ||\\ vv5# /||        ", 22);
	printfLine("  || \\___ / ||        ", 23);
	printfLine("  ||        ||         ", 24);
}


void os::osakaFace() {

	printfLine("                 @@@@@@@@@@@@@@@@@@@", 0);
	printfLine("             @@@@@@@@@@@@@@@@@@@@@@@@@@@", 1);
	printfLine("          @@@@@@@@@@@@@@@@v@@@@@@@@@@@@@@@@", 2);
	printfLine("        @@@@@@@@@@@@@@@@@v v@@@@@@@@@@@@@@@@@", 3);
	printfLine("       @@@@@@@@@@@v@@@@@v   v@@@@@v@@@@@@@@@@@", 4);
	printfLine("      @@@@@@@@@@@v v@@@v     v@@@v v@@@@@@@@@@@", 5);
	printfLine("     @@@@@@@@vv@v   v@v       v@v   v@vv@@@@@@@@", 6);
	printfLine("     @@@@@@@v _v_____v         v ____v_ v@@@@@@@", 7);
	printfLine("     @@@@@@v /  *****\\         /*****  \\ v@@@@@@", 8); 
	printfLine("     @@@@@@v/  *******         *******  \\v@@@@@@", 9);
	printfLine("     @@@@@@ |  *******         *******  | @@@@@@              saka", 10);
	printfLine("     @@@@@@ \\  *******         *******  / @@@@@@", 11);
	printfLine("     @@@@@@     *****           *****     @@@@@@", 12);
	printfLine("     @@@@@@                               @@@@@@", 13);
	printfLine("     @@@@@@                               @@@@@@", 14);
	printfLine("     @@@@@@@            _____            @@@@@@@", 15);
	printfLine("     v@@@@@@@@          \\___/          @@@@@@@@v", 16);
	printfLine("     v@@@@@@@@@@@@@               @@@@@@@@@@@@@v", 17);
	printfLine("      v@@@@@@@@@@@@@@@@_______@@@@@@@@@@@@@@@@v", 18);
	printfLine("      v@@@@@@@@@@@@@@@@|     |@@@@@@@@@@@@@@@@v", 19);
	printfLine("       v@@@@@@@@@@@@@@_|     |_@@@@@@@@@@@@@@v", 20);
	printfLine("       v@@@@@@@@@@@@ / \\_____/ \\ @@@@@@@@@@@@v", 21);
	printfLine("        v@@@@@@@@@@ /    \\ /    \\ @@@@@@@@@@v", 22);
	printfLine("         v@v v@v v@|      |      |@v v@v v@v", 23);
	printfLine("          v   v   v|      |      |v   v   v", 24);
}
	
void os::osakaHead() {
  	
  	printfLine("                               @@@@@@@@@@@@@", 0);
  	printfLine("                          @@@@@@@@@@@@@@@@@@@@@@@", 1);
  	printfLine("                       @@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 2);
  	printfLine("                    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 3);
  	printfLine("                   @@@@@@@@@@@@@@@@@@v@@@@@@@@@@@@@@@@@@", 4);
  	printfLine("                 @@@@@@@@@@@@@@@@@@@v v@@@@@@@@@@@@@@@@@@@", 5);
  	printfLine("                @@@@@@@@@@@@@@@@@@@v   v@@@@@@@@@@@@@@@@@@@", 6);
  	printfLine("               @@@@@@@@@@@@@@@@@@@v     v@@@@@@@@@@@@@@@@@@@", 7);
  	printfLine("              @@@@@@@@@@@@@v v@@@v       v@@@v v@@@@@@@@@@@@@", 8);
  	printfLine("             @@@@@@@@@@@@@v___@@v_       _v@@___v@@@@@@@@@@@@@", 9);
  	printfLine("             @@@@@@@@@@@@v    @v           v@    v@@@@@@@@@@@@", 10);
  	printfLine("            @@@@@@@@@ v@v     v             v     v@v @@@@@@@@@", 11);
  	printfLine("            @@@@@@@@v  v                           v  v@@@@@@@@", 12);
  	printfLine("           @@@@@@@@v                                   v@@@@@@@@", 13);
  	printfLine("           @@@@@@@@ ___~~~~~~~~~~~       ~~~~~~~~~~~___ @@@@@@@@", 14);
  	printfLine("          @@@@@@@@v                                     v@@@@@@@@", 15);
  	printfLine("          @@@@@@@@                                       @@@@@@@@", 16);
  	printfLine("          @@@@@@@@                                       @@@@@@@@", 17);
  	printfLine("         @@@@@@@@@                                       @@@@@@@@@", 18);
  	printfLine("         @@@@@@@@@                                       @@@@@@@@@", 19);
  	printfLine("         @@@@@@@@@_                                     _@@@@@@@@@", 20);
  	printfLine("  ,,,,,,,,,,,,@@@@@\\_              _____              _/@@@@@,,,,,,,,,,,,", 21);
  	printfLine(",,,           ,,_@@@@\\_            \\___/            _/@@@@_,,           ,,,", 22);
  	printfLine(",,  \\   \\   \\  ,,\\=====\\_                         _/=====/,,  /   /   /  ,,", 23);
  	printfLine("  ,,,,,,,,,,,,,, /       \\_______________________/       \\ ,,,,,,,,,,,,,,", 24);
}

void os::osakaKnife() {
	
	printfLine("            .~@@@@@@~.", 0);
	printfLine("           #@@@@@@@@@@&", 1);
	printfLine("          $@@@@@@@@@@@@0", 2);
	printfLine("         /@@@@@     @@@@#", 3);
	printfLine("         @@@@ __   __ @@@@", 4);
	printfLine("        :@@@ /==   ==\\@@@", 5);
	printfLine("        #@@@    ___   @@@", 6);
	printfLine("        @@@@\\_  \\_/  @@@:", 7);
	printfLine("        @@@@| \\____/@@@/   \\", 8);
	printfLine("       /  @@|   ||\\@@/v     \\", 9);
	printfLine("      /\\  v@ \\_/ |/\\v        \\", 10);
	printfLine("     :| |  v_/|\\/ | |:        \\  line of", 11);
	printfLine("     [| ;     |   ; |]         \\  sight", 12);
	printfLine("      =|      |    |=           \\   |", 13);
	printfLine("      ||      |    ||            \\  |", 14);
	printfLine("      ||      |    ||             \\ v", 15);
	printfLine("      || ____ | ___||              \\", 16);
	printfLine("      {| |__| | |_|||               \\", 17);
	printfLine("      ||\\  ___|____||___             \\", 18);
	printfLine("      <~(=|       _____/ <---         \\", 19);
	printfLine("       ~~~|____~~~  ~       |          \\", 20);
	printfLine("       5[!   : :    :  master maison    \\", 21);
	printfLine("       5[!   : :    : santoku knife 8\"   \\", 22);
	printfLine("       5[!   : :    :  premium german", 23);
	printfLine("       5[!   : :    :  stainless steel     ? <---- everything you hold sacred", 24);
}


void os::cat() {
	
	printfLine("                                              @@@       @@@                      ", 0);
	printfLine("                                             @   @     @   @                     ", 1);
	printfLine("                                            @     @   @     @                    ", 2);
	printfLine("                                           @       @ @       @                   ", 3);
	printfLine("        hello                             @         @         @                  ", 4);
	printfLine("      everynyan                          @                     @                 ", 5);
	printfLine("                                        @                       @                ", 6);
	printfLine("                                        @  -~^~-       -~^~-    @                ", 7);
	printfLine("                                       @  {  |  }     {  |  }    @               ", 8);
	printfLine("                                       @   -~_~-       -~_~-     @               ", 9);
	printfLine("                                      @         ^~^~^             @              ", 10);
	printfLine("                                      @         |   |             @              ", 11);
	printfLine("                 ~~__                 @         \\___/             @              ", 12);
	printfLine("                 ~~~~~~_             @                             @             ", 13);
	printfLine("                    ~~~~~_         _~@                             @~~__        _", 14);
	printfLine("                       ~~~~~~~~~~~~~~@                             @~~~~~~~~~~~~~", 15);
	printfLine("                         ~~~~~~~~~~~ @                             @    ~~~~~~~~ ", 16);
	printfLine("                              ~~     @                             @             ", 17);
	printfLine("                                     @                            #@             ", 18);
	printfLine("                                     @                           ##@             ", 19);
	printfLine("                                      @                         ##@              ", 20);
	printfLine("                                      @#                       ###@              ", 21);
	printfLine("                                      @##                    #####@              ", 22);
	printfLine("                                       @##                  #####@               ", 23);
	printfLine("                                        @###               #####@                ", 24);
}



void os::god() {
	
	printfLine("                                                                                 ", 0);
	printfLine("                                               @@@@@@@@@@@@@@                    ", 1);
	printfLine("                                            @@@@@@@@@@@@@@@@@@@@                 ", 2);
	printfLine("                                          @@@@@@@@@@@@@@@@@@@@@@@                ", 3);
	printfLine("                                         @@@@@@vv@v  v@@@v@@@@@@@@               ", 4);
	printfLine("                                        @@@@@@v  v    v@v v@@@@@@@@              ", 5);
	printfLine("                                        @@@@ v__ \\    /v __v @@@@@@              ", 6);
	printfLine("          OH MY GAH                    /@@@@ /  \\       /  \\  @@@@@\\             ", 7);
	printfLine("                                       @@@@  \\__/       \\__/  @@@@@@             ", 8);
	printfLine("                                       @@@@                   @@@@@@             ", 9);
	printfLine("                                       @@@@                   @@@@@@             ", 10);
	printfLine("                                       @@@@       _____       @@@@@@             ", 11);
	printfLine("                                       @@@@      |     \\      @@@@@@             ", 12);
	printfLine("                                       @@@@\\__   \\_____/    _/@@@@@@             ", 13);
	printfLine("                                      /@@@@@@@\\___      ___/@@@@@@@@\\            ", 14);
	printfLine("                                      @@@@@@@@@@@@\\____/@@@@@@@@@@@@@            ", 15);
	printfLine("                                      @@@@@v@@v__/|    |  \\_@@@@@@@@@            ", 16);
	printfLine("                                      @@v@v v@/   |    |     \\@@@@v@@            ", 17);
	printfLine("                                      @v v_/      |    |        @v_@@            ", 18);
	printfLine("                                      v /         \\____/        v   v            ", 19);
	printfLine("                                       $\\___      |ssss|       _____/$           ", 20);
	printfLine("                                      $sssssv____ \\ssss/  ____vsssssss$          ", 21);
	printfLine("                                     $sssssssssss\\_|ss|__/sssssssssssss$         ", 22);
	printfLine("                                    $sssssssssssssss&&sssssssssssssssssss$       ", 23);
	printfLine("                                  $sssssssssssssssss||ssssssssssssssssssss$      ", 24);
}
