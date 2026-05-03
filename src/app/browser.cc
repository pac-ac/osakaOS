#include <app/browser.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::filesystem;
using namespace os::net;


void sleep(uint32_t);
uint8_t* memset(uint8_t*, int, size_t);
uint8_t valCount(uint8_t*, uint8_t, size_t);
uint16_t prng();





Shinosaka::Shinosaka(Network* network) {

	this->appType = APP_TYPE_SHINOSAKA;
	this->net = network;
	this->memoryManager = this->net->filesystem->memoryManager;

	for (int i = 0; i < 64; i++) { this->domainName[i] = '\0'; }
	for (int i = 0; i < 16; i++) { this->addressStr[i] = '\0'; }
}

Shinosaka::~Shinosaka() {
}


void Shinosaka::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {
	
	if (!init) {
	
		this->DrawTheme(widget);
		init = true;
	}
		

	if (this->showScrollBar) {

		gc->FillRectangle(widget->x+widget->w-8, widget->y+(10*!(widget->Fullscreen)), 8, 
				widget->h-(10*!(widget->Fullscreen)), W9A9A9A);
	
		gc->FillRectangle(widget->x+widget->w-8, widget->y+(10*!(widget->Fullscreen)), 8, 
				(int)((float)(widget->h-(10*!(widget->Fullscreen)))*this->pageScroll), WCACACA);
	
		gc->DrawLineFlat(widget->x+widget->w-8, widget->y+(10*!(widget->Fullscreen)), 
				widget->x+widget->w-8, widget->y+widget->h-(10*!(widget->Fullscreen)), W656565, false);
	}
	
	
	if (this->sendFlagDNS) {
	
		//this->net->requestID = widget->ID;
		this->SendRequestDNS(widget);
		this->sendFlagDNS = false;
	}
	
	if (this->sendFlagLocalHTTP) {
		
		//free memory
		if (this->htmlBuffer != nullptr) { 	   this->memoryManager->free(this->htmlBuffer); }
		if (this->httpResponseBuffer != nullptr) { this->memoryManager->free(this->httpResponseBuffer); }
	
		this->httpResponseBuffer = nullptr;
		this->responseLength = 0;
		this->htmlBuffer = nullptr;
		this->htmlBufferSize = 0;
		this->packetsReceived = 0;
		
		this->sendFlagLocalHTTP = false;
	
		widget->textColor = W000000;

		if (strcmp(this->domainName, "localhost/")) {
		
			this->ReadInput(this->domainName + strlen("localhost/"), widget, this->net->filesystem);
		} else {
			this->RenderHTML(widget, this->net->fileData, this->net->fileDataSize);
		}
		widget->textColor = WFFFFFF;
	}
	
	if (this->sendFlagHTTP) {
		
		//free memory
		if (this->htmlBuffer != nullptr) { 	   this->memoryManager->free(this->htmlBuffer); }
		if (this->httpResponseBuffer != nullptr) { this->memoryManager->free(this->httpResponseBuffer); }

		this->httpResponseBuffer = nullptr;
		this->responseLength = 0;
		this->htmlBuffer = nullptr;
		this->htmlBufferSize = 0;
		this->packetsReceived = 0;
		
		widget->textColor = W0000FF;
		widget->Print("Sending HTTP request...\n", TEXT_ITALIC);
		widget->textColor = WFFFFFF;
		
		//this->net->requestID = widget->ID;
		this->SendRequestHTTP(widget);

		//home network media
		//if (this->udpSocket == nullptr) {

			//this->udpSocket = this->net->udp->Connect(str2ip(this->domainName), HNM_PORT);
			//this->udpSocket->Send((uint8_t*)"GET---HNM", 9);
		//}
		//this->net->hnm->SendRequest();
		
		this->sendFlagHTTP = false;
	}
				
	
	//if (net->requestID == widget->ID) {
				
		//render html documents	
		if (this->tcpSocket != nullptr) {
			
			/*
			widget->textColor = W000000;
			
			for (int i = 0; i < tcpSocket->bufferIndex; i++) {
				
				widget->PutChar((char)(tcpSocket->handleBuffer[i]));
			}
			widget->PutChar('\n');
			widget->textColor = WFFFFFF;
			*/
		
			if (tcpSocket->handleType == HANDLE_FLAG_TCP) {
		
				widget->textColor = W00FF00;
				widget->Print("HTTP message received.\n", TEXT_BOLD);
				widget->textColor = WFFFFFF;
	
				this->packetsReceived++;
				
				//if (this->htmlBufferSize >= this->contentLength+this->responseLength) {	
				if (this->AllocateMemoryHTML(tcpSocket->handleBuffer, tcpSocket->bufferIndex) == 0) {
			
					this->pageScroll = 1.0;
					this->firstRender = true;
					
					widget->textColor = W000000;
					this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
					widget->textColor = WFFFFFF;
					
					this->tcpSocket->handleType = HANDLE_FLAG_EMPTY;
					net->EmptyHandleBufferTCP(this->tcpSocket);
					
					//net->tcp->Disconnect(this->tcpSocket);
					//this->tcpSocket = nullptr;
				}
				//widget->textColor = W000000;
				//this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
				//widget->textColor = WFFFFFF;
			}

			if (tcpSocket->connectionFail) {
			
				widget->textColor = WFF5555;
				widget->Print("Failed to establish TCP connection\n", TEXT_BOLD);
				widget->textColor = WFFFFFF;
				tcpSocket->connectionFail = false;
			}
		}
	
		
		if (this->udpSocket != nullptr) {
				
			/*	
			widget->textColor = W000000;
			
			for (int i = 0; i < udpSocket->bufferIndex; i++) {
				
				widget->PutChar((char)(udpSocket->handleBuffer[i]));
			}
			widget->PutChar('\n');
			widget->textColor = WFFFFFF;
			*/

			if (udpSocket->handleType == HANDLE_FLAG_UDP) {
					
				if (this->dns_id != 0) {

					//get ip from dns response
					if (this->HandleResponseDNS(udpSocket->handleBuffer, udpSocket->bufferIndex, widget)) {
					
						udpSocket->handleType = HANDLE_FLAG_EMPTY;
						net->EmptyHandleBufferUDP(udpSocket);
						this->sendFlagHTTP = true;
					
					//home network media
					} else {
						widget->FillBuffer(0, 0, widget->gc->gfxWidth, widget->gc->gfxHeight, net->HNM_Buffer);
						for (int i = 0; i < (udpSocket->bufferIndex < BUFFER_SIZE_13H ? udpSocket->bufferIndex : BUFFER_SIZE_13H); i++) {
							widget->buf[i] = udpSocket->handleBuffer[i];
						}	
						udpSocket->handleType = HANDLE_FLAG_EMPTY;
						net->EmptyHandleBufferUDP(udpSocket);
					}
				}
			}
		}
	//}
	App::ComputeAppState(gc, widget);
}


void Shinosaka::DrawTheme(CompositeWidget* widget) {
	
	widget->DrawRectangle(0, 0, widget->gc->gfxWidth, widget->gc->gfxHeight, W1C0071, true);
	
	char* fileName = "BROWSER_INIT_IMG";

	if (this->net->filesystem->FileIf(net->filesystem->GetFileSector(fileName))) {
	
		uint8_t tmp[widget->gc->gfxBufferSize];
		uint16_t w = widget->gc->gfxWidth;
		uint16_t h = widget->gc->gfxHeight;
		uint8_t* ptr = nullptr;

		if (this->net->filesystem->GetTagFile("compressed", net->filesystem->GetFileSector(fileName), ptr)) {
		
			this->net->filesystem->Read13H(fileName, tmp, &w, &h, true);
		} else {
			this->net->filesystem->Read13H(fileName, tmp, &w, &h, false);
		}

		for (uint32_t y = 0; y < widget->gc->gfxHeight; y++) {
		
			for (uint32_t x = 0; x < widget->gc->gfxWidth; x++) {
			
				widget->WritePixel(x, y, tmp[widget->gc->gfxWidth*y+x]);
			}
		}
	}
	
	widget->textColor = WFF0000;
	widget->Print("Shinosaka Web Browser\n", TEXT_BOLD);
	widget->textColor = WFFFFFF;
	widget->Print("This is a web utility to make HTTP 1.1 requests for\n");
	widget->Print("basic websites and rendering them as HTML pages.\n");
	widget->Print("Press tab to access the window menu and put in the\ndomain name and address for the web server.\n");
	widget->Print("\nThis program is still in critical development.\n", TEXT_ITALIC);
	widget->textColor = WFFFFFF;
}



void Shinosaka::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {

	uint8_t offsety = 18;
	if (widget->Fullscreen) { offsety = 9; }

	uint8_t offsetx = 2;
	if (this->showScrollBar) { offsetx = 9; }
	
	gc->MakeDark(2, widget->x+1, widget->y+offsety, widget->w-offsetx, widget->h-offsety);
	/*
	gc->FillRectangle(widget->x, widget->y+offsety, widget->w, 10, WAAAAAA);
	gc->DrawRectangle(widget->x, widget->y+offsety, widget->w, 10, W555555);
	gc->DrawLineFlat(widget->x, widget->y+offsety, widget->x, 10, WAAAAAA, false);
	*/
	
	gc->FillRectangle(widget->x, widget->y+offsety, widget->w-offsetx+2, 20, WAAAAAA);
	gc->DrawRectangle(widget->x, widget->y+offsety, widget->w-offsetx+2, 20, W555555);
	gc->DrawLineFlat(widget->x, widget->y+offsety, widget->x-offsetx+2, 20, WAAAAAA, false);

	switch (this->keyTarget) {
	
		case 1:
			gc->FillRectangle(widget->x+1, widget->y+offsety+10, widget->w-offsetx, 10, W0000FF);
			break;
		default:
			gc->FillRectangle(widget->x+1, widget->y+offsety, widget->w-offsetx, 10, W0000FF);
			break;
	}
	
	gc->PutText("Domain Name: ", widget->x+1, widget->y+offsety+1, WFFFFFF);
	gc->PutText(this->domainName, widget->x+(6*13), widget->y+offsety+1, WFFFFFF);
	
	gc->PutText("Host Address: ", widget->x+1, widget->y+offsety+11, WFFFFFF);
	gc->PutText(this->addressStr, widget->x+(6*14), widget->y+offsety+11, WFFFFFF);


	if (this->httpResponseBuffer != nullptr) {
	
		gc->PrintData((char*)this->httpResponseBuffer, this->responseLength, 
				widget->x+1, widget->y+offsety+21, WFFFFFF);
	}

	gc->PutText("Size in B: ", widget->x+1, widget->y+widget->h-10, WFFFFFF);
	gc->PutText(int2str(this->htmlBufferSize), widget->x+68, widget->y+widget->h-10, WFFFFFF);

	gc->PutText("Packets Rcvd: ", widget->x+160, widget->y+widget->h-10, WFFFFFF);
	gc->PutText(int2str(this->packetsReceived), widget->x+252, widget->y+widget->h-10, WFFFFFF);
}



void Shinosaka::SaveOutput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {

	if (this->htmlBuffer == nullptr) {
	
		widget->Print("No data found in HTML buffer.\n");
	}

	uint8_t data[OFS_BLOCK_SIZE];

	if (filesystem->FileIf(filesystem->GetFileSector(fileName)) == false) {

		filesystem->NewFile(fileName, data, OFS_BLOCK_SIZE);	
	}

	//save html data to file
	for (int i = 0; i < (this->htmlBufferSize/OFS_BLOCK_SIZE)+1; i++) {
	
		for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
	
			if (OFS_BLOCK_SIZE*i+j < this->htmlBufferSize) {
			
				data[j] = this->htmlBuffer[OFS_BLOCK_SIZE*i+j];
			}
		}
		filesystem->WriteLBA(fileName, data, i);
	}
}


void Shinosaka::ReadInput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(filesystem->GetFileSector(fileName))) {

		this->htmlBufferSize = filesystem->GetFileSize(fileName);
	
		if (this->htmlBuffer != nullptr) {
		
			this->memoryManager->free(this->htmlBuffer);
			this->htmlBuffer = nullptr;
		}

		this->htmlBuffer = (uint8_t*)this->memoryManager->malloc(sizeof(uint8_t)*this->htmlBufferSize);
		uint8_t data[OFS_BLOCK_SIZE];

		
		for (int i = 0; i < (this->htmlBufferSize/OFS_BLOCK_SIZE); i++) {
		
			filesystem->ReadLBA(fileName, data, i);

			for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
			
				if (i+j < this->htmlBufferSize) {
				
					this->htmlBuffer[i+j] = data[i+j];
				}
			}
		}
		this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
	} else {
		
	}
}

bool Shinosaka::Connect2Server(uint32_t ip) {

	if (this->tcpSocket != nullptr) {
	
		this->net->tcp->Disconnect(tcpSocket);
		this->tcpSocket = nullptr;
	}

	//port 80 is the default http port
	this->tcpSocket->poll = false;
	this->tcpSocket = this->net->tcp->Connect(ip, 80);
	this->net->tcp->Bind(this->tcpSocket, this->net);
	//sleep(600);

	return (this->tcpSocket != nullptr);
}


void Shinosaka::SendRequestDNS(CompositeWidget* widget) {
	
	if (this->udpSocket != nullptr) {
	
		this->net->udp->Disconnect(this->udpSocket);
		this->udpSocket = nullptr;
	}
	
	uint8_t labelCount = valCount((uint8_t*)this->domainName, (uint8_t)'.', strlen(this->domainName));	
	size_t messageLengthDNS = sizeof(DomainNameSystemHeader) + strlen(this->domainName) + labelCount + 5;
	
	this->dns_id = prng();

	uint8_t* messageDNS = (uint8_t*)this->memoryManager->malloc(messageLengthDNS);
	memset(messageDNS, 0x00, messageLengthDNS);


	//set id and flags etc.
	messageDNS[0] = this->dns_id >> 8;
	messageDNS[1] = this->dns_id & 0xff;
	messageDNS[2] = 0x01;
	messageDNS[5] = 1;
	messageDNS[messageLengthDNS-3] = 0x01;
	messageDNS[messageLengthDNS-1] = 0x01;

	

	uint8_t labelLength = 0;
	uint8_t labelIndex = sizeof(DomainNameSystemHeader);

	//put domain name in query section
	for (int i = 0; i < strlen(this->domainName)+1; i++) {
	
		if (this->domainName[i] == '.' || this->domainName[i] == '\0') {
		
			messageDNS[labelIndex] = labelLength;
			labelLength = 0;
			labelIndex += i+1;
		} else {
			messageDNS[i+sizeof(DomainNameSystemHeader)+1] = this->domainName[i];
			labelLength++;
		}
	}


	widget->textColor = W000000;
	for (int i = 0; i < messageLengthDNS; i++) { 
		
		int asciiNum = '0' * (messageDNS[i] < 32);
		widget->PutChar(messageDNS[i]+asciiNum); 
	}
	widget->PutChar('\n'); 
	widget->textColor = WFFFFFF;

	this->udpSocket = this->net->udp->Connect(str2ip("8.8.8.8"), DNS_PORT);
	this->net->udp->Bind(this->udpSocket, this->net);
	this->udpSocket->Send((uint8_t*)messageDNS, messageLengthDNS);

	

	this->memoryManager->free(messageDNS);
}

//dns message is being sent, but no response is heard
uint16_t Shinosaka::HandleResponseDNS(uint8_t* data, uint16_t dataSize, CompositeWidget* widget) {
	
	/*	
	if (this->dns_id != ((data[0] << 8) | data[1])) {
	
		return 0;
	}
	*/
	
	uint16_t numOfAnswers = ((data[6] << 8) | data[7]);
	uint32_t answerIndex = sizeof(DomainNameSystemHeader);


	//get domain ip from response
	for (int i = 0; i < numOfAnswers; i++) {
	
		uint16_t recordOffset =  strlen((char*)data+answerIndex) + 1;
		uint16_t rdlength = 		data[recordOffset+answerIndex+8] << 8 | 
						data[recordOffset+answerIndex+9];
		
		//get host name	
		for (int j = 0; j < recordOffset; j++) {
			
			this->domainName[j] = data[answerIndex+j];
		}
		this->domainName[recordOffset+answerIndex] = '\0';


		//ip provided
		if (rdlength == 4) {
		
			this->domainIP = (
				(data[recordOffset+answerIndex+10] << 24) | 
				(data[recordOffset+answerIndex+11] << 16) |
				(data[recordOffset+answerIndex+12] << 8) |
				 data[recordOffset+answerIndex+13]);
			break;

		//some other domain provided
		} else {
			for (int j = 0; j < rdlength; j++) {
			
				this->domainName[j] = data[recordOffset+answerIndex+10+j];
			}
			this->domainName[recordOffset+answerIndex+10+rdlength] = '\0';
			answerIndex += rdlength+recordOffset+10;
			this->domainIP = 0;
		}
	}
	
	this->dns_id = 0;
	this->net->udp->Disconnect(this->udpSocket);
	this->udpSocket = nullptr;
	
	return this->domainIP;
}



void Shinosaka::SendRequestHTTP(CompositeWidget* widget) {

	if (this->Connect2Server(this->domainIP)) {

		/*
		//seperate file request from domain name
		uint16_t pathIndex = 0;
		uint16_t pathSize = 0;

		for (int i = 0; i < strlen(this->domainName); i++) {
		
			if (this->domainName[i] == '/') {
				
				pathIndex = i + 1;
				pathSize = strlen(this->domainName)-pathIndex;
				this->domainName[i] = '\0';
			}
		}
		*/


		//construct request
		//char* baseRequest = "GET /";
		//char* fileRequest = " HTTP/1.1\r\nHost: ";
		
		
		char* baseRequest = "GET / HTTP/1.1\r\nHost: ";
		char* endRequest = "\r\nUser-Agent: osakaOS\r\nAccept: */*\r\nConnection: close\r\n\r\n";
		
		uint32_t requestSize = strlen(baseRequest) + strlen(this->domainName) + strlen(endRequest) + 1;
		//uint32_t requestSize = strlen(baseRequest) + pathSize + strlen(fileRequest) + strlen(this->domainName) + strlen(endRequest) + 1;
		char* request = (char*)this->memoryManager->malloc(sizeof(char)*requestSize);
		
		int i = 0;
		int j = 0;

		/*
		if (pathIndex != 0) {
			
			for (i; i < strlen(baseRequest); i++) { 	request[i] = baseRequest[i]; }
			j = i;
			for (i; i < pathSize; i++) { 			request[i] = this->domainName[pathIndex+(i-j)]; }
			j = i;
			for (i; i < strlen(fileRequest); i++) { 	request[i] = fileRequest[i-j]; }
			j = i;
		} else {
			for (i; i < strlen(fullBaseRequest); i++) { 	request[i] = fullBaseRequest[i]; }
			j = i;
		}
		*/
			
		for (i; i < strlen(baseRequest); i++) { 	request[i] = baseRequest[i]; }
		j = i;
		
		for (i; i < j+strlen(this->domainName); i++) {	request[i] = this->domainName[i-j]; }
		j = i;
		for (i; i < requestSize-1; i++) {		request[i] = endRequest[i-j]; }
		request[requestSize-1] = '\0';
		
		this->tcpSocket->Send((uint8_t*)request, strlen(request));
		//this->net->tcp->Disconnect(this->tcpSocket);
		
		this->memoryManager->free(request);
		
		//this->domainName[pathIndex-1] = '/';
	} else {
		widget->Print("\nFailed to connect to server at ");
		widget->Print(this->domainName);
		widget->Print("\n.");
	}
}


uint16_t Shinosaka::HandleResponseHTTP(uint8_t* data, uint16_t dataSize) {

	//store response header
	for (int i = 0; i < dataSize-5; i++) {

		if (data[i] == '\r' &&   data[i+1] == '\n' &&
		    data[i+2] == '\r' && data[i+3] == '\n') {
		
			this->responseLength = i+4;
			if (this->httpResponseBuffer != nullptr) {
			
				this->memoryManager->free(this->httpResponseBuffer);
			}
			this->httpResponseBuffer = (uint8_t*)this->memoryManager->malloc(sizeof(uint8_t)*this->responseLength);
			break;
		}
	}
	
	for (int i = 0; i < this->responseLength; i++) {
	
		this->httpResponseBuffer[i] = data[i];
	}

	//get content length
	char lengthStr[16];
	uint16_t lengthIndex = 0;
	char* searchStr = "Content-Length: ";

	for (int i = 0; i < this->responseLength; i++) {
	
		if (strcmp(searchStr, (char*)this->httpResponseBuffer+i)) {
		
			lengthIndex = i+strlen(searchStr);
			
			int j = lengthIndex;
			for (j; this->httpResponseBuffer[j] != '\r'; j++) {
			
				lengthStr[j-lengthIndex] = this->httpResponseBuffer[j];
			}
			lengthStr[j-lengthIndex] = '\0';
			this->contentLength = str2int(lengthStr);
			break;
		}
	}
	return HTTP_STATUS_200;
}


bool Shinosaka::CheckIfConnectionDone(uint8_t* data, uint16_t dataSize) {

	if (this->packetCheckBuf == nullptr) {
	
		this->packetCheckBuf = (uint8_t*)this->memoryManager->malloc(sizeof(uint8_t)*dataSize);
		for (int i = 0; i < dataSize; i++) {
		
			this->packetCheckBuf[i] = data[i];
			this->packetCheckSize = dataSize;
		}
	} else {
		if (dataSize != this->packetCheckSize) { return false; }

		for (int i = 0; i < packetCheckSize; i++) {
		
			//if new data still being sent
			if (data[i] != this->packetCheckBuf[i]) {
			
				return false;
			}
		}
	}
	this->memoryManager->free(this->packetCheckBuf);
	this->packetCheckBuf = nullptr;
	this->packetCheckSize = 0;


	if (this->tcpSocket != nullptr) {

		this->net->tcp->Disconnect(this->tcpSocket);
		this->tcpSocket = nullptr;
	}

	return true;
}


uint16_t Shinosaka::AllocateMemoryHTML(uint8_t* dataBuffer, uint32_t bufferSize) {
	
	
	//parse and save header seperately
	bool firstResponse = false;
	if (this->responseLength == 0) {
	
		firstResponse = true;
		this->HandleResponseHTTP(dataBuffer, bufferSize);
	}
		

	uint8_t* tempBuffer = (uint8_t*)this->memoryManager->malloc(this->htmlBufferSize + bufferSize);	
	
	for (int index = 0; index < (this->htmlBufferSize + bufferSize); index++) {

		if (index < this->htmlBufferSize && this->htmlBuffer != nullptr)	{
			
			tempBuffer[index] = this->htmlBuffer[index];
		} else {tempBuffer[index] = dataBuffer[index-this->htmlBufferSize]; }
	}

	if (this->htmlBuffer != nullptr) { this->memoryManager->free(this->htmlBuffer); }

	this->htmlBuffer = tempBuffer;
	this->htmlBufferSize += bufferSize;

	
	if (firstResponse) {
	
		for (int i = 0; i < this->htmlBufferSize; i++) {
	
			this->htmlBuffer[i] = this->htmlBuffer[i+this->responseLength];
		}
		this->htmlBufferSize -= this->responseLength;
	}

	//stop loading sites larger than 2 MB for now	
	if (this->htmlBufferSize >= 2*1024*1024) {

		char* mem_error_msg = "\n!osakaOS: OUT_OF_BROWSER_MEMORY";
		
		for (int i = 0; i < strlen(mem_error_msg); i++) {
		
			this->htmlBuffer[htmlBufferSize-strlen(mem_error_msg)+i] = mem_error_msg[i];
		}
		return 0;
	}
	
	return 0;
}


TreeNodeDOM* Shinosaka::CreateTreeNode(uint16_t elementType) {

	TreeNodeDOM* node = (TreeNodeDOM*)this->memoryManager->malloc(sizeof(TreeNodeDOM));

	node->elementType = elementType;
	node->children = (List*)this->memoryManager->malloc(sizeof(List));
	new (node) List(this->memoryManager);

	return node;
}

uint16_t Shinosaka::CreateTreeDOM() {

	this->documentRoot = CreateTreeNode(HTML_TAG_NONE);

	uint32_t tagIndex = 0;
	uint32_t tagLayers = 0;
	bool tagReading = false;

	uint16_t elementType = 0;

	for (int i = 0; i < this->htmlBufferSize; i++) {
	
		switch (this->htmlBuffer[i]) {
		
			case '<':
				tagIndex = i;
				tagReading = true;
				break;
			case '>':
				{
					uint8_t tagLength = i - tagIndex;
					char tagStr[tagLength];
				
					int j = 0;
					for (j; j < tagLength-1; j++) {
					
						tagStr[j] = this->htmlBuffer[tagIndex+j+1];
					}
					tagStr[j] = '\0';

					elementType = this->GetTagHTML(tagStr);
				

					if (elementType == HTML_TAG_END && tagLayers > 0) {
					
						tagLayers--;
					} else {
						tagLayers++;
					}
				}
				break;
			default:
				break;
		}
	}
}



uint8_t Shinosaka::GetTagHTML(char* tagStr) {

	if (tagStr[0] == '/') { 		return HTML_TAG_END; }

	if (strcmp(tagStr, "html")) {		return HTML_TAG_HTML;
	} else if (strcmp(tagStr, "head")) {	return HTML_TAG_HEAD;
	} else if (strcmp(tagStr, "title")) {	return HTML_TAG_TITLE;
	} else if (strcmp(tagStr, "body")) {	return HTML_TAG_BODY;
	}


	switch (tagStr[0]) { 
	
		case 'h':
			return HTML_TAG_HEADER;
			break;
		case 'i':
			return HTML_TAG_ITALIC;
			break;
		case 'b':
			return HTML_TAG_BOLD;
		case 'a':
			return HTML_TAG_LINK;
			break;
		case 'p':
			if (strlen(tagStr) < 2) {
				return HTML_TAG_PARAGRAPH;
			}
			break;
		default:
			break;
	}
	return HTML_TAG_NONE;
}


void Shinosaka::DumpHTML(CompositeWidget* widget, uint8_t* dataBuffer, uint32_t bufferSize) {
	
	for (int i = 0; i < (int)((float)bufferSize * (this->pageScroll)); i++) {
	
		widget->PutChar((char)dataBuffer[i]);
	}
}

void Shinosaka::RenderHTML(CompositeWidget* widget, uint8_t* dataBuffer, uint32_t bufferSize) {

	uint32_t tagStart = 0;
	uint32_t tagEnd = 0;
	uint32_t tagValue = HTML_TAG_NONE;
	uint32_t prevTagValue = HTML_TAG_NONE;
	
	uint32_t charsPrinted = 0;
	uint32_t linesPrinted = 0;
	
	bool readingTag = false;
	char title[64];
	uint8_t titleIndex = 0;
	memset((uint8_t*)title, 0, 64);
	
	widget->Print("\v");

	for (int i = 0; i < (int)((float)bufferSize * (this->pageScroll)); i++) {

		switch (dataBuffer[i]) {
	
			case 0x00:
				break;
			case '<':
				tagStart = i;
				readingTag = true;
				break;
			case '>':
				tagEnd = i;
				{
					uint8_t tagLength = tagEnd - tagStart;
					char tagStr[tagLength];
				
					int j = 0;
					for (j; j < tagLength-1; j++) {
					
						tagStr[j] = dataBuffer[tagStart+j+1];
					}
					tagStr[j] = '\0';
				
					prevTagValue = tagValue;
					tagValue = this->GetTagHTML(tagStr);

					if (tagValue == HTML_TAG_END) {
					
						if (prevTagValue == HTML_TAG_PARAGRAPH) {	
											widget->PutChar('\n');
						} else if (prevTagValue == HTML_TAG_HEADER) {	
											widget->PutChar('\n');
											widget->PutChar('\n');
											linesPrinted++;
						} else if (prevTagValue == HTML_TAG_TITLE) {	
											widget->UpdateName(title);
											titleIndex = 0;
											memset((uint8_t*)title, 0, 64);	
						} else {
							widget->PutChar('\n');
						}
						linesPrinted++;
					}
				}
				readingTag = false;
				break;
			default:
				if (readingTag == false && tagValue != HTML_TAG_END) {
				
					if (tagValue ==        HTML_TAG_PARAGRAPH) { widget->PutChar((char)dataBuffer[i]);
					} else if (tagValue == HTML_TAG_ITALIC) {    widget->PutChar((char)dataBuffer[i], TEXT_ITALIC);
					} else if (tagValue == HTML_TAG_BOLD) {	     widget->PutChar((char)dataBuffer[i], TEXT_BOLD);
					} else if (tagValue == HTML_TAG_HEADER) {    widget->PutChar((char)dataBuffer[i], TEXT_HEADER);
										     charsPrinted++;
					} else if (tagValue == HTML_TAG_LINK) {
						
						widget->textColor = W0000FF;
						widget->PutChar((char)dataBuffer[i]);
						widget->textColor = W000000;
					
					} else if (tagValue == HTML_TAG_TITLE) {
						
						if (titleIndex < 64) {
						
							title[titleIndex] = (char)dataBuffer[i];
							titleIndex++;
						}
					} else {
						widget->PutChar((char)dataBuffer[i]);
					}
					charsPrinted++;

					if (charsPrinted >= widget->currentTextWidth) {
					
						charsPrinted = 0;
						linesPrinted++;
					}
				}
				break;
		}

		if (linesPrinted >= widget->currentTextHeight && firstRender) {
		
			this->firstRender = false;
			this->pageScroll = ((float)i)/((float)bufferSize);
			return;
		}
	}
}


void Shinosaka::OnKeyDown(char ch, CompositeWidget* widget) {

	char* strTarget = nullptr;
	uint8_t* strIndex = nullptr;
	uint8_t maxStrLength = 0;

	switch (this->keyTarget) {
	
		case 1:
			maxStrLength = 16;
			strIndex = &(this->addressStrIndex);
			strTarget = this->addressStr;
			break;
		default:
			maxStrLength = 64;
			strIndex = &(this->domainNameIndex);
			strTarget = this->domainName;
			break;
	}
	
	switch (ch) {

		case '\xfd':
			if (widget->MenuOpen) {
				
				this->keyTarget ^= 1;
			
			} else if (pageScroll >= 0.1 && this->htmlBuffer != nullptr) {
				
				this->pageScroll -= 0.01;
				widget->textColor = W000000;
				this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
				widget->textColor = WFFFFFF;
			}
			break;
		
		case '\xfe':
			if (widget->MenuOpen) {
				
				this->keyTarget ^= 1;
			
			} else if (pageScroll <= 1.0 && this->htmlBuffer != nullptr) { 
				
				this->pageScroll += 0.01; 
				widget->textColor = W000000;
				this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
				widget->textColor = WFFFFFF;
			}
			
			break;
		case '\n':
			//send http request
			if (*strIndex > 0 && this->sendFlagHTTP == false) {
					
				strTarget[*strIndex] = '\0';
				this->domainIP = str2ip(this->addressStr);
	
				//localhost testing
				if (this->net->activateServerHTTP && (this->domainIP == str2ip("127.0.0.1") || strcmp(this->domainName, "localhost"))) {

					this->sendFlagLocalHTTP = true;
				} else {
					this->sendFlagHTTP = true;
				}
			}
			
			//send dns request
			/*
			if (this->domainNameIndex > 0) {
	
				this->domainName[this->domainNameIndex] = '\0';
				this->sendFlagDNS = true;
			}
			*/
			break;
		case '\b':
			if (*strIndex > 0 && widget->MenuOpen) {
			
				(*strIndex)--;
				strTarget[*strIndex] = '\0';
			}
			break;
		default:
			if (*strIndex < maxStrLength && this->sendFlagHTTP == false && widget->MenuOpen) {

				strTarget[*strIndex] = ch;
				(*strIndex)++;
				strTarget[*strIndex] = '\0';
			}
			break;
	}
}

void Shinosaka::OnKeyUp(char ch, CompositeWidget* widget) {
}



void Shinosaka::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
		
	uint8_t offsety = 9;
	//if (widget->Fullscreen) { offsety = 9; }

	if (widget->MenuOpen) {
	
		if (x < widget->x+widget->w-9 && y > widget->y+offsety && y < widget->y+offsety+10) {

			this->keyTarget = 0;
		
		} else if (x < widget->x+widget->w-9 && y > widget->y+offsety+10 && y < widget->y+offsety+20) {
			
			this->keyTarget = 1;
		}
	}
	


	//page scroll
	if (x > widget->x+widget->w-10) { 
	//if (x > widget->x+widget->w-8 && x <= widget->x+widget->w-2) { 
			
		this->pageScroll = (((float)(y-widget->y)) / (float)(widget->h-(((10*!(widget->Fullscreen))+offsety))));
		this->scrolling = true;
		
		widget->textColor = W000000;
		this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
		widget->textColor = WFFFFFF;
	} else { 
		widget->Dragging = true; 
	}
}

void Shinosaka::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
			
	this->scrolling = false;
}

void Shinosaka::OnMouseMove(int32_t oldx, int32_t oldy, 
			int32_t newx, int32_t newy, 
			CompositeWidget* widget) {

	if (newx > widget->x+widget->w-10) {

		this->showScrollBar = true;
	
		if (this->scrolling) {
		
			uint8_t offsety = 18;
			if (widget->Fullscreen) { offsety = 9; }
		
			this->pageScroll = (((float)(newy-widget->y)) / (float)(widget->h-((10*!(widget->Fullscreen))+offsety))); 
			
			widget->textColor = W000000;
			this->RenderHTML(widget, this->htmlBuffer, this->htmlBufferSize);
			widget->textColor = WFFFFFF;
		}
	} else {
		this->showScrollBar = false;
	}
}
