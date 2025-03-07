#include <boost/asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

const std::string IMAGE_DIR = "Media";
const std::string HOST = "http://localhost:8080/";

std::string get_mime_type(const std::string &filename) {
    if (filename.ends_with(".jpg") || filename.ends_with(".jpeg"))
        return "image/jpeg";

    if (filename.ends_with(".png"))
        return "image/png";

    return "application/octet-stream";
}

std::string read_file(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void handle_request(tcp::socket &socket) {
    try {
        boost::asio::streambuf buffer;
        boost::system::error_code error;
        boost::asio::read_until(socket, buffer, "\r\n\r\n", error);

        if (error && error != boost::asio::error::eof) {
            std::cerr << "Read error: " << error.message() << std::endl;
            return;
        }

        // Convert buffer to string
        std::istream request_stream(&buffer);
        std::string request_line;
        std::getline(request_stream, request_line);

        std::cout << "Received request: " << request_line << std::endl;

        // Extract requested file path
        std::istringstream iss(request_line);
        std::string method, url, http_version;
        iss >> method >> url >> http_version;

        // Remove leading '/'
        if (url.starts_with("/")) {
            url = url.substr(1);
        }

        // Construct full file path
        std::string file_path = IMAGE_DIR + "/" + url;

        if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
            std::string file_content = read_file(file_path);
            std::string mime_type = get_mime_type(file_path);

            // Send HTTP response with image data
            std::string header = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: " +
                                 mime_type +
                                 "\r\n"
                                 "Content-Length: " +
                                 std::to_string(file_content.size()) +
                                 "\r\n"
                                 "\r\n";

            boost::asio::write(socket, boost::asio::buffer(header));
            boost::asio::write(socket, boost::asio::buffer(file_content));
        } else {
            // File not found response
            std::string response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "\r\n"
                                   "404 Not Found";

            boost::asio::write(socket, boost::asio::buffer(response));
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        std::cout << "Server started at " << HOST << std::endl;
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            handle_request(socket);
        }
    } catch (std::exception &e) {
        std::cerr << "Error; " << e.what() << std::endl;
    }

    return 0;
}
