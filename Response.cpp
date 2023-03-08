#include "Response.hpp"

using namespace webserv;

Response::Response()
{
    cout << "RESPONSE CONSTRUCTOR CALLED\n";
}

Response::Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it)
{
    _req = req;
    _root_path = root_path;
    _it = it;
    this->respond();
}

void Response::respond(void)
{
    const char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"; // Dynamically add content length TODO
    const char *header2 = "HTTP/1.1 200 OK\nContent-Type: image/*\r\n\r\n";
    const char *header_404 = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n";

    std::ifstream myfile;
    string entireText;
    string line;

    if (_req.path() == "/")
        myfile.open(_root_path + "/index.html", std::ios::binary);
    else
        myfile.open(_root_path + _req.path(), std::ios::binary);
    if (!myfile)
    {
        entireText += header_404;
        myfile.open("public/404.html", std::ios::binary);
    }
    else
    {
        if (_req.path() == "/edlim.jpg" || _req.path() == "/edlim_lrg.jpg" || _req.path() == "/jng.png")
            entireText += header2;
        else
            entireText += header;
    }

    char read_buffer[65535]; // create a read_buffer
    while (myfile.read(read_buffer, sizeof(read_buffer)))
        entireText.append(read_buffer, myfile.gcount());
    entireText.append(read_buffer, myfile.gcount());
    myfile.close();
    if (_req.done())
    {
        if (_req.path() == "/upload.html")
            _req.process_image();
        int total_to_send = entireText.length();
        cout << "total to send :" << total_to_send << endl;
        for (ssize_t total_sent = 0; total_sent < total_to_send;)
        {
            size_t len = (entireText.length() > 50000 ? 50000 : entireText.length());
            string temp = entireText.substr(0, len);
            int sent = send(_it->first, temp.c_str(), len, 0);
            if (sent == -1)
            {
                cout << "errno :" << errno << endl;
                break;
            }
            entireText = entireText.substr(len);
            total_sent += sent;
            cout << "length sent :" << sent << endl;
            cout << "total sent :" << total_sent << endl;
        }
    }
}
