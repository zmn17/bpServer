FROM ubuntu:22.04

# install dependecies
RUN apt update && apt install -y g++ cmake make libboost-all-dev

# set the working directory in the container
WORKDIR /app

# Copy the source code into the container
COPY server.cpp .

# Copy the images
COPY Media /app/Media/

# Compile the server
RUN g++ -std=c++20 -o server server.cpp -lpthread -lboost_system

# Expose port 8080
EXPOSE 8080

# run the server
CMD ["./server"]




