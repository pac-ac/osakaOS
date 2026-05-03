#ifndef __OS__APP__BROWSER_H
#define __OS__APP__BROWSER_H


#include <common/types.h>
#include <gui/window.h>
#include <filesys/ofs.h>
#include <drivers/amd_am79c973.h>
#include <net/network.h>
#include <list.h>


#define DNS_PORT 53


#define HTML_TAG_NONE 0
#define HTML_TAG_END 1

#define HTML_TAG_HTML 2
#define HTML_TAG_HEAD 3
#define HTML_TAG_TITLE 4
#define HTML_TAG_BODY 5
#define HTML_TAG_TEXT 6

#define HTML_TAG_PARAGRAPH 7
#define HTML_TAG_HEADER 8
#define HTML_TAG_ITALIC 9
#define HTML_TAG_BOLD 10
#define HTML_TAG_LINK 11


#define HTTP_STATUS_200 200
#define HTTP_STATUS_404 404
#define HTTP_STATUS_500 500

#define MAX_PACKET_BUFFER_NUM 128


namespace os {


	struct DomainNameSystemHeader {
	
		common::uint16_t id;
		common::uint16_t flag;
		common::uint16_t numOfQuestions;
		
		//for query, all below is 0
		common::uint16_t numOfAnswers;
		common::uint16_t numOfRR;
		common::uint16_t numOfRR2;

	} __attribute__((packed));


	struct TreeNodeDOM {
	
		common::uint16_t elementType;
		List* children;
	
	} __attribute__((packed));


	class Shinosaka : public App {

		public:
			net::Network* net;
			net::TransmissionControlProtocolSocket* tcpSocket = nullptr;
			net::UserDatagramProtocolSocket* udpSocket = nullptr;

			MemoryManager* memoryManager = nullptr;
			bool init = false;

			
			common::uint8_t* httpResponseBuffer = nullptr;
			common::uint16_t responseLength = 0;
			common::uint32_t contentLength = 0;
			
			
			common::uint8_t* htmlBuffer = nullptr;
			common::uint32_t htmlBufferSize = 0;
			bool scrolling = false;
			float pageScroll = 0.0;

			
			common::uint8_t* packetCheckBuf = nullptr;
			common::uint16_t packetCheckSize = 0;
			
			common::uint16_t packetsReceived = 0;

			TreeNodeDOM* documentRoot = nullptr;
			
			bool sendFlagHTTP = false;
			bool sendFlagLocalHTTP = false;
			bool sendFlagDNS = false;

			common::uint16_t dns_id = 0;
			common::uint32_t domainIP = 0;
			
			char domainName[64];
			common::uint8_t domainNameIndex = 0;
			char addressStr[16];
			common::uint8_t addressStrIndex = 0;

			bool firstRender = false;
			bool showScrollBar = false;
			common::uint8_t keyTarget = 0;


			char* webFileName = nullptr;
			common::uint32_t serverIP = 0;
		public:
			Shinosaka(net::Network* network);
			~Shinosaka();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			virtual void SaveOutput(char* fileName, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);
			virtual void ReadInput(char* fileName, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);

			virtual void DrawTheme(gui::CompositeWidget* widget);

			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);

			bool Connect2Server(common::uint32_t ip);
			
			void SendRequestHTTP(gui::CompositeWidget* widget);
			common::uint16_t HandleResponseHTTP(common::uint8_t* data, common::uint16_t dataSize);
			bool CheckIfConnectionDone(common::uint8_t* data, common::uint16_t dataSize);
			
			void SendRequestDNS(gui::CompositeWidget* widget);
			common::uint16_t HandleResponseDNS(common::uint8_t* data, common::uint16_t dataSize, gui::CompositeWidget* widget);
			
			
			common::uint16_t AllocateMemoryHTML(common::uint8_t* dataBuffer, common::uint32_t bufferSize);
			TreeNodeDOM* CreateTreeNode(common::uint16_t elementType);
			common::uint16_t CreateTreeDOM();
			common::uint8_t GetTagHTML(char* tagStr);
			void DumpHTML(gui::CompositeWidget* widget, common::uint8_t* dataBuffer, common::uint32_t bufferSize);
			void RenderHTML(gui::CompositeWidget* widget, common::uint8_t* dataBuffer, common::uint32_t bufferSize);

			virtual void OnKeyDown(char ch, gui::CompositeWidget* widget);
			virtual void OnKeyUp(char ch, gui::CompositeWidget* widget);

			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy, 
						gui::CompositeWidget* widget);
	};
}










#endif
