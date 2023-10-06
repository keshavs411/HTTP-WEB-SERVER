#include<tmwp>
#include<stdio.h>
#include<windows.h>
#include<string.h>
#include<iostream>
#include<stdlib.h>
using namespace std;
using namespace tmwp;

void Request::setValue(string k,string v)
{
keyValues.insert(pair<string,string>(k,v));
}
string Request::getValue(string k)
{
map<string,string>::iterator i;
i=keyValues.find(k);
if(i==keyValues.end()) return string("");
else return string(i->second);
}
string Request::get(string name)
{
string val;
int i,e;
for(i=0;i<this->dataCount;i++)
{
for(e=0;data[i][e]!='\0' && data[i][e]!='=';e++);
if(data[i][e]!='=') continue ;//back to next cyle of loop
if(strncmp(data[i],name.c_str(),e)==0) break;
}
if(i==this->dataCount)
{
val="";
}
else
{
val=string(data[i]+(e+1));
}
cout<<val<<endl;
return val;
}
void Request::forward(string forwardTo)
{
this->forwardTo=forwardTo;
}

Response::Response(int clientSocketDescriptor)
{
this->clientSocketDescriptor=clientSocketDescriptor;
this->isClosed=false;
this->headerCreated=false; 
}
void Response::createHeader()
{
char header[51];
strcpy(header,"HTTP/1.1 200 OK\nContent-Type:text/html\n\n");
send(clientSocketDescriptor,header,strlen(header),0);
this->headerCreated=true;
}
void Response::write(const char *str)
{
if(str==NULL) return;
int len=strlen(str);
if(len==0) return;
if(!this->headerCreated) createHeader();
send(this->clientSocketDescriptor,str,len,0);
}

void Response ::write(string str)
{
this->write(str.c_str());
}
void Response::close()
{
if(this->isClosed) return;
closesocket(this->clientSocketDescriptor);
this->isClosed=true;
}

int extensionEquals(const char *left,const char *right)
{
char a,b;
while(*left && *right)
{
a=*left;
b=*right;
if(a>=65 && a<=90) a+=32;
if(b>=65 && b<=90) b+=32;
if(a!=b) return 0;
left++;
right++;
}
return *left==*right;
}
char *getMIMEType(char *resource)
{
char *mimeType;
mimeType=NULL;
int len=strlen(resource);
if(len<4)  return mimeType;
int lastIndexOfDot=len-1;
while(lastIndexOfDot>0 && resource[lastIndexOfDot]!='.') lastIndexOfDot--;
if(extensionEquals(resource+lastIndexOfDot+1,"html"))
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"text/html");
}
if(extensionEquals(resource+lastIndexOfDot+1,"css"))
{
mimeType=(char *)malloc(sizeof(char)*9);
strcpy(mimeType,"text/css");
}
if(extensionEquals(resource+lastIndexOfDot+1,"js"))
{
mimeType=(char *)malloc(sizeof(char)*16);
strcpy(mimeType,"text/javscript");
}
if(extensionEquals(resource+lastIndexOfDot+1,"jpeg"))
{
mimeType=(char *)malloc(sizeof(char)*11);
strcpy(mimeType,"image/jpeg");
}
if(extensionEquals(resource+lastIndexOfDot+1,"jpg"))
{
mimeType=(char *)malloc(sizeof(char)*11);
strcpy(mimeType,"image/jpeg");
}
if(extensionEquals(resource+lastIndexOfDot+1,"png"))
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"image/png");
}
if(extensionEquals(resource+lastIndexOfDot+1,"ico"))
{
mimeType=(char *)malloc(sizeof(char)*13);
strcpy(mimeType,"image/x=icon");
}
return mimeType;
}
char isClientSideTechnologyResource(char *resource)
{
int i;
for(i=0;resource[i]!='\0' && resource[i]!='.';i++);
if(resource[i]=='\0') return 'N';
return 'Y';
}
Request * parseRequest(char *bytes)
{
char method[11];
char resource[1001];
int i,j;
for(i=0;bytes[i]!=' ';i++) method[i]=bytes[i];
method[i]='\0';
i+=2;
char **data=NULL;
int dataCount=0;
if(strcmp(method,"GET")==0)
{
//whatever?djbsbjdfb=djsfjsdbf&djfsnj=fdgfsn&jkds=ndkjs
for(j=0;bytes[i]!=' ';j++,i++)
{
if(bytes[i]=='?') break;
resource[j]=bytes[i];
}
resource[j]='\0';
if(bytes[i]=='?')
{
i++;
int si=i;
while(bytes[i]!=' ')
{
if(bytes[i]=='&') dataCount++;
i++;
}
dataCount++;
data=(char **)malloc(sizeof(char *)*dataCount);
int *pc=(int *)malloc(sizeof(int)*dataCount);
i=si;
int j=0;
while(bytes[i]!=' ')
{
if(bytes[i]=='&')
{
pc[j]=i;
j++;
}
i++;
}
pc[j]=i;
i=si;
j=0;
int howManyToPick;
while(j<dataCount)
{
howManyToPick=pc[j]-i;
data[j]=(char*)malloc(sizeof(char )*howManyToPick+1);
strncpy(data[j],bytes+i,howManyToPick);
data[j][howManyToPick]='\0';
i=pc[j]+1;
j++;
}
}
}
printf("Request arrived for %s\n",resource);
Request *request=new Request;
request->dataCount=dataCount;
request->data=data;
request->method=(char *)malloc((sizeof(char )*strlen(method))+1);
strcpy(request->method,method);
if(resource[0]=='\0')
{
request->resource=NULL;
request->isClientSideTechnologyResource='Y';
request->mimeType=NULL;
}
else
{
request->resource=(char *)malloc((sizeof(char )*strlen(resource))+1);
strcpy(request->resource,resource);
request->isClientSideTechnologyResource=isClientSideTechnologyResource(resource);
request->mimeType=getMIMEType(resource);
}
return request;
}
TMWebProjector::TMWebProjector(int portNumber)
{
this->portNumber=portNumber;
}
TMWebProjector::~TMWebProjector()
{
}
void TMWebProjector::onRequest(string url,void (*ptrOnRequest) (Request & request,Response &response))
{
if(url.length()==0 || ptrOnRequest==NULL) return;
RequestMappings.insert(pair<string,void (*)(Request &,Response &)>(url,ptrOnRequest));
}
  
       
void TMWebProjector::start()
{
FILE *f;
char g;
int length,rc;
int i,ByteExtract;
char responseBuffer[1024];
char requestBuffer[8193];
WORD ver;
WSADATA wsaData;
int serverSocketDescriptor;
int clientSocketDescriptor;
struct sockaddr_in serverSocketInformation;
struct sockaddr_in clientSocketInformation;
int successCode;
int len;
ver=MAKEWORD(1,1);//for version and its 2byte
WSAStartup(ver,&wsaData);//socket library
serverSocketDescriptor=socket(AF_INET,SOCK_STREAM,0);
if(serverSocketDescriptor<0)
{
printf("Unable to create socket.\n");
return;
}
serverSocketInformation.sin_family=AF_INET;
serverSocketInformation.sin_port=htons(this->portNumber);//host to network in short
serverSocketInformation.sin_addr.s_addr=htonl(INADDR_ANY);
successCode=bind(serverSocketDescriptor,(struct sockaddr *)&serverSocketInformation,sizeof(serverSocketInformation));
char message[101];
if(successCode<0)
{
sprintf(message,"Unable to bind socket to port  %d",this->portNumber);
printf("%s\n",message);
WSACleanup();
return;
}
listen(serverSocketDescriptor,0);
len=sizeof(clientSocketInformation);
while(1)
{
sprintf(message,"Server is ready to accept request on port %d ",this->portNumber);
printf("%s\n",message);
clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&clientSocketInformation,&len);
if(clientSocketDescriptor<0)
{
printf("Unable to accept client connection\n");
closesocket(serverSocketDescriptor);
WSACleanup();
return;
}
ByteExtract=recv(clientSocketDescriptor,requestBuffer,8192,0);
if(ByteExtract<0)
{

}else if(ByteExtract==0)
{

}
else 
{
requestBuffer[ByteExtract]='\0';
Request *request=parseRequest(requestBuffer);
while(1) //infinite loop introduced because of forwarding feature
{
if(request->isClientSideTechnologyResource=='Y')
{
if(request->resource==NULL)
{
f=fopen("index.html","rb");
if(f!=NULL) printf("Sending index.html\n");
if(f==NULL) 
{
f=fopen("index.htm","rb");
if(f!=NULL) printf("Sending index.htm\n");
}
if(f==NULL) 
{
strcpy(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:163\nclose:connection\n\n<!DOCTYPE HTML><html lang='eg'><head><meta charset='utf-8'><title>Tm Web Projector</title></head><body><h2 style='color:red'>Resource / not found</h2></body></html>");
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer)+1,0);
printf("Sending 404 page\n");
break; //break introduced beacause of forwarding feature
}
else
{
fseek(f,0,2);//move the internal pointer  to the end of file
length=ftell(f);
fseek(f,0,0);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\nclose:connection\n\n",length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(i<length)
{
rc=length-i;
if(rc>1024) rc=1024;
fread(responseBuffer,rc,1,f);
send(clientSocketDescriptor,responseBuffer,rc,0);
i+=rc;
}
}
fclose(f);
closesocket(clientSocketDescriptor);
break; //break introduced beacause of forwarding feature
}
else
{
f=fopen(request->resource,"rb");
if(f!=NULL) printf("Sending %s\n",request->resource);
if(f==NULL) 
{
char tmp[501];
printf("Sending 404 page\n");
sprintf(tmp,"<!DOCTYPE HTML><html lang='eg'><head><meta charset='utf-8'><title>Tm Web Projector</title></head><body><h2 style='color:red'>Resource /%s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\nclose:connection\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer)+1,0);
closesocket(clientSocketDescriptor);
break; //break introduced beacause of forwarding feature
}
else
{
fseek(f,0,2);//move the internal pointer  to the end of file
length=ftell(f);
fseek(f,0,0);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:%s\nContent-Length:%d\nclose:connection\n\n",request->mimeType,length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(i<length)
{
rc=length-i;
if(rc>1024) rc=1024;
fread(responseBuffer,rc,1,f);
send(clientSocketDescriptor,responseBuffer,rc,0);
i+=rc;
}
fclose(f);
closesocket(clientSocketDescriptor);
break; //break introduced beacause of forwarding feature

}
}
}
else
{
map<string,void(*)(Request &,Response &)>::iterator i;
i=RequestMappings.find(string("/")+string(request->resource));
if(i==RequestMappings.end())
{
printf("Sending 404 page\n");
char tmp[501];
printf("Sending 404 page\n");
sprintf(tmp,"<!DOCTYPE HTML><html lang='eg'><head><meta charset='utf-8'><title>Tm Web Projector</title></head><body><h2 style='color:red'>Resource /%s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\nclose:connection\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer)+1,0);
break; //break introduced beacause of forwarding feature
}
else
{
Response response(clientSocketDescriptor);
i->second(*request,response);
if(request->forwardTo.length()>0)
{
free(request->resource);
request->resource=(char *)malloc(sizeof(char )*request->forwardTo.length());
strcpy(request->resource,request->forwardTo.c_str());
request->isClientSideTechnologyResource=isClientSideTechnologyResource(request->resource);
request->mimeType=getMIMEType(request->resource);
request->forwardTo="";
continue;
}
if(request->data!=NULL)
{
for(int k=0;k<request->dataCount;k++) free(request->data[k]);
free(request->data);
}

break; //break introduced beacause of forwarding feature
}
}
}
}
}//infinite loop ends here
WSACleanup();
return;
}