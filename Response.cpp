#include "Response.hpp"

using namespace webserv;

Response::Response()
{
    _hasText = false;
}

Response::Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it) : _req(req), _root_path(root_path), _it(it), _hasText(true)
{
    this->readFile();
}

Response::~Response()
{
    
}

bool Response::hasText(void) { return (_hasText); }

void Response::readFile(void)
{
    const char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"; // Dynamically add content length TODO
    const char *header2 = "HTTP/1.1 200 OK\nContent-Type: image/*\r\n\r\n";
    const char *header_404 = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n";

    std::ifstream myfile;

    if (_req.path() == "/")
        myfile.open(_root_path + "/index.html", std::ios::binary);
    else
        myfile.open(_root_path + _req.path(), std::ios::binary);
    if (!myfile)
    {
        _entireText += header_404;
        myfile.open("public/404.html", std::ios::binary);
    }
    else
    {
        if (_req.path().find(".jpg") != string::npos || _req.path().find(".png") != string::npos)
            _entireText += header2;
        else
            _entireText += header;
    }

    char read_buffer[65535]; // create a read_buffer
    // string test;
    // while (getline(myfile, test))
        // _entireText += test;
    while (myfile.read(read_buffer, sizeof(read_buffer)))
        _entireText.append(read_buffer, myfile.gcount());
    // cout << _entireText << endl;
    // cout << "========" << endl;
    _entireText.append(read_buffer, myfile.gcount());
    // cout << "========" << endl;
    // cout << _entireText << endl;
    // cout << _entireText.length();
    myfile.close();
    // exit(0);
    // if (_req.path().find(".jpg") != string::npos || _req.path().find(".png") != string::npos)
    //     exit(0);
}

void Response::respond(void)
{
    // cout << "I"
    if (_req.done())
    {
        if (_req.path() == "/upload.html")
            _req.process_image();
        int total_to_send = _entireText.length();
        // cout << "total to send :" << total_to_send << endl;
        for (ssize_t total_sent = 0; total_sent < total_to_send;)
        {
            size_t len = (_entireText.length() > 100000 ? 100000 : _entireText.length());
            string to_send = _entireText.substr(0, len);
            int sent = send(_it->first, to_send.c_str(), len, 0);
            if (sent == -1)
            {
                std::cout << "total sent bytes: " << total_sent << endl;
                std::cout << "total To send: " << total_to_send << endl;
                cout << "errno :" << errno << endl;
                break;
            }
            _entireText = _entireText.substr(len);
            total_sent += sent;
            // std::cout << "total sent bytes: " << total_sent << endl;
            // std::cout << "total To send: " << total_to_send << endl;
            if (total_sent == total_to_send)
                _hasText = false;
            // cout << "length sent :" << sent << endl;
            // cout << "total sent :" << total_sent << endl;
        }
    }
}
