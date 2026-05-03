#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <sys/stat.h>


#define WIDTH_13H 320
#define HEIGHT_13H 200


using namespace std;


/* to write BMP images to OFS files export them in gimp in 24 bit with "do not write color space info" checked */



//osakaOS default VGA palette
constexpr uint32_t defaultPalette[] = {

	0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF, 
	0x000000, 0x101010, 0x202020, 0x353535, 0x454545, 0x555555, 0x656565, 0x757575, 0x8A8A8A, 0x9A9A9A, 0xAAAAAA, 0xBABABA, 0xCACACA, 0xDFDFDF, 0xEFEFEF, 0xFFFFFF, 
	0x0000FF, 0x4100FF, 0x8200FF, 0xBE00FF, 0xFF00FF, 0xFF00BE, 0xFF0082, 0xFF0041, 0xFF0000, 0xFF4100, 0xFF8200, 0xFFBE00, 0xFFFF00, 0xBEFF00, 0x82FF00, 0x41FF00,
	0x00FF00, 0x00FF41, 0x00FF82, 0x00FFBE, 0x00FFFF, 0x00BEFF, 0x0082FF, 0x0041FF, 0x8282FF, 0x9E82FF, 0xBE82FF, 0xDF82FF, 0xFF82FF, 0xFF82DF, 0xFF82BE, 0xFF829E, 
	0xFF8282, 0xFF9E82, 0xFFBE82, 0xFFDF82, 0xFFFF82, 0xDFFF82, 0xBEFF82, 0x9EFF82, 0x82FF82, 0x82FF9E, 0x82FFBE, 0x82FFDF, 0x82FFFF, 0x82DFFF, 0x82BEFF, 0x829EFF, 
	0xBABAFF, 0xCABAFF, 0xDFBAFF, 0xEFBAFF, 0xFFBAFF, 0xFFBAEF, 0xFFBADF, 0xFFBACA, 0xFFBABA, 0xFFCABA, 0xFFDFBA, 0xFFEFBA, 0xFFFFBA, 0xEFFFBA, 0xDFFFBA, 0xCAFFBA, 
	0xBAFFBA, 0xBAFFCA, 0xBAFFDF, 0xBAFFEF, 0xBAFFFF, 0xBAEFFF, 0xBADFFF, 0xBACAFF, 0x000071, 0x1C0071, 0x390071, 0x550071, 0x710071, 0x710055, 0x710039, 0x71001C, 
	0x710000, 0x711C00, 0x713900, 0x715500, 0x717100, 0x557100, 0x397100, 0x1C7100, 0x007100, 0x00711C, 0x007139, 0x007155, 0x007171, 0x005571, 0x003971, 0x001C71, 
	0x393971, 0x453971, 0x553971, 0x613971, 0x713971, 0x713961, 0x713955, 0x713945, 0x713939, 0x714539, 0x715539, 0x716139, 0x717139, 0x617139, 0x557139, 0x457139, 
	0x397139, 0x397145, 0x397155, 0x397161, 0x397171, 0x396171, 0x395571, 0x394571, 0x515171, 0x595171, 0x615171, 0x695171, 0x715171, 0x715169, 0x715161, 0x715159, 
	0x715151, 0x715951, 0x716151, 0x716951, 0x717151, 0x697151, 0x617151, 0x597151, 0x517151, 0x517159, 0x517161, 0x517169, 0x517171, 0x516971, 0x516171, 0x515971, 
	0x000041, 0x100041, 0x200041, 0x310041, 0x410041, 0x410031, 0x410020, 0x410010, 0x410000, 0x411000, 0x412000, 0x413100, 0x414100, 0x314100, 0x204100, 0x104100, 
	0x004100, 0x004110, 0x004120, 0x004131, 0x004141, 0x003141, 0x002041, 0x001041, 0x202041, 0x282041, 0x312041, 0x392041, 0x412041, 0x412039, 0x412031, 0x412028, 
	0x412020, 0x412820, 0x413120, 0x413920, 0x414120, 0x394120, 0x314120, 0x284120, 0x204120, 0x204128, 0x204131, 0x204139, 0x204141, 0x203941, 0x203141, 0x202841, 
	0x2D2D41, 0x312D41, 0x352D41, 0x3D2D41, 0x412D41, 0x412D3D, 0x412D35, 0x412D31, 0x412D2D, 0x41312D, 0x41352D, 0x413D2D, 0x41412D, 0x3D412D, 0x35412D, 0x31412D, 
	0x2D412D, 0x2D4131, 0x2D4135, 0x2D413D, 0x2D4141, 0x2D3D41, 0x2D3541, 0x2D3141, 0x0055AA, 0xAA0055, 0xAA5555, 0xAAAA55, 0xFF5500, 0xFF55AA, 0xFFAA00, 0xFFAA55

};


int GetFileSize(string fileName) {

	struct stat stat_buf;
	int rc = stat(fileName.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

uint8_t Web2VGA(uint32_t color) {

	float Rc = (color >> 16);
	float Gc = ((color >> 8) & 0xff);
	float Bc = (color & 0xff);
	
	float closest = (255.0*255.0)*3.0 + 1.0;

	uint8_t index = 0;

	for (int i = 0; i < 256; i++) {
	
		float Rp = (defaultPalette[i] >> 16);
		float Gp = ((defaultPalette[i] >> 8) & 0xff);
		float Bp = (defaultPalette[i] & 0xff);
		
		float euclidDist = ((Rp-Rc)*(Rp-Rc)) + 
				   ((Gp-Gc)*(Gp-Gc)) +
				   ((Bp-Bc)*(Bp-Bc));
		
		if (closest > euclidDist) {
		
			closest = euclidDist;
			index = i;
		
		} else if (euclidDist < 0.05) {
		
			index = i;
			break;
		}
	}
	return index;
}


uint8_t* ReadFileBMP(const char* filename, uint16_t* retWidth, uint16_t* retHeight) {

	FILE* file = fopen(filename, "rb");
	uint8_t header[54];

	fread(header, sizeof(uint8_t), 54, file);

	int width = *(int*)&header[18];
	int height = *(int*)&header[22];

	//check if image fits image resolution
	if (width > WIDTH_13H || height > HEIGHT_13H) {
	
		cerr << "BMP image is too large for 320x200 VGA resolution." << endl;
		return nullptr;
	}
	*retWidth = width;
	*retHeight = height;

	//allocate and read in pixel data
	int size = 3 * width * height;
	uint8_t* data = new uint8_t[size];

	fread(data, sizeof(uint8_t), size, file);

	for (int i = 0; i < size; i += 3) {
	
		uint8_t tmp = data[i];
		data[i] = data[i+2];
		data[i+2] = tmp;
	}
	fclose(file);
	return data;
}


uint8_t* Convert2OFS(uint8_t* data, uint32_t sizeBMP) {
	
	uint8_t* newData = new uint8_t[sizeBMP/3];

	for (int i = 0; i < sizeBMP; i += 3) {
	
		uint32_t fullPixelVal = (data[i] << 16) 
					| (data[i+1] << 8) 
					| (data[i+2]);

		newData[i/3] = Web2VGA(fullPixelVal);
	}
	delete[] data;
	
	return newData;
}


int main(int argc, char* argv[]) {

	if (argc < 3) {
	
		cerr << "Please pass the file for the VM image[1], the file you want written[2], and the OFS file[3] to write to." << endl;
		return -1;
	}
	const char* vm_image = argv[1];
	const char* exfile = argv[2];
	const char* infile = argv[3];

	
	//check if file to be written is appropriate size
	int fileSize = GetFileSize(exfile);
	cout << exfile << " is " << (fileSize/1024) << "KB in size." << endl;
	
	//max file size will be added is 32MB
	if (fileSize > (32*1024*1024)) {
	
		cerr << "Cannot add files 32MB in size or greater." << endl;
		return -2;
	}


	//check for bmp image to 
	//format and add as OFS image
	uint8_t* dataBMP = nullptr;
	uint16_t width = WIDTH_13H;
	uint16_t height = HEIGHT_13H;
	string findStr = exfile;
	
	if (findStr.find(".bmp") != string::npos) {
	
		cout << "BMP image found, formatting to OFS image data." << endl;
		uint8_t* tmpBMP = ReadFileBMP(exfile, &width, &height);
		dataBMP = Convert2OFS(tmpBMP, 3*width*height);
		fileSize = width*height; 
		cout << "Image is of width " << width << "px and height " << height << "px." << endl;
		cout << "BMP OFS image is " << fileSize << "B in size." << endl;
	}

	

	//set up search string	
	char OFS_File_Header_Str[40];
	for (int i = 0; i < 40; i++) { OFS_File_Header_Str[i] = 0x00; }

	//ofs magic numbers
	OFS_File_Header_Str[0] = 0xF1;
	OFS_File_Header_Str[1] = 0x7E;
	
	//size of file
	OFS_File_Header_Str[5] = 0x08;
	
	for (int i = 8; (argv[3])[i-8] != '\0'; i++) {
	
		OFS_File_Header_Str[i] = (argv[3])[i-8];
	}
	string searchStr = OFS_File_Header_Str;
	
	
	

	//search for string in image file
	ifstream diskImageFile(vm_image);
	string line;
	int currentLine = 0;
	int lineIndex = 0;
	size_t linePos = 0;
	
	if (!diskImageFile.is_open()) {
	
		cerr << "Unable to open image file " << vm_image << "." << endl;
		return -3;
	}


	//temp file for writing data up until search line
	ofstream outputFile("temp.img");

	if (outputFile.is_open() == false) {
	
		cerr << "Failed to create file for writing." << endl;
		return -4;
	}

	
	//find start of filesystem in image file
	while (getline(diskImageFile, line)) {
	
		currentLine++;
		linePos = line.find(searchStr);
	
		if (linePos != string::npos) {
		
			lineIndex = currentLine;
			break;
		}
		outputFile << line << endl;
	}
	cout << "OFS file found in line " << lineIndex << " at offset " << linePos << "." << endl;



	
	//modify current line to with external file data
	ifstream externalFile(exfile);
	string readLine;
	

	//modify header
	uint32_t blockSize = ((fileSize / 2048) + 1) * 2048;
	uint32_t dataSize = fileSize;
	
	line[linePos+4] = (blockSize)       & 0xff;
	line[linePos+5] = (blockSize >> 8)  & 0xff;
	line[linePos+6] = (blockSize >> 16) & 0xff;
	line[linePos+7] = (blockSize >> 24);
	line[linePos+40] = (dataSize)       & 0xff;
	line[linePos+41] = (dataSize >> 8)  & 0xff;
	line[linePos+42] = (dataSize >> 16) & 0xff;
	line[linePos+43] = (dataSize >> 24);
	
	//image size/resolution
	line[linePos+124] = width >> 8;
	line[linePos+125] = width & 0xff;
	line[linePos+126] = height >> 8;
	line[linePos+127] = height & 0xff;

	linePos += 512;	


	//add external data to virtual disk image
	if (dataBMP == nullptr) {
	
		while (getline(externalFile, readLine)) {

			for (int i = 0; i < readLine.length(); i++) {
		
				line[linePos] = readLine[i];
				linePos++;
			}
		}
		outputFile << line << endl;
		externalFile.close();

	//add OFS formatted bmp image
	} else {
		//for (int i = 0; i < fileSize; i++) {
	
		//??? wahtever	
		//for (int y = 0; y < height; y++) {
		for (int y = height-1; y >= 0; y--) {
			for (int x = 0; x < width; x++) {	
			
				//line[linePos] = dataBMP[i];
				line[linePos] = dataBMP[width*y+x];
				linePos++;
			}
		}
		outputFile << line << endl;
		externalFile.close();
	}

	
	//add rest of file to virtual disk image
	currentLine = 0;
	
	while (getline(diskImageFile, line)) {
			
		outputFile << line << endl;
	
		/*
		currentLine++;
		
		if (currentLine > lineIndex) {
		
			outputFile << line << endl;
		}
		*/
	}

	diskImageFile.close();
	outputFile.close();


	//remove original file and rename temp to be original
	if (remove(vm_image) != 0) {
	
		cerr << "Failed to delete original image." << endl;
		return -5;
	}
	
	if (rename("temp.img", vm_image) != 0) {
		
		cerr << "Failed to rename new image to original." << endl;
		return -6;
	}

	cout << "File data added to image." << endl;

	return 0;
}
