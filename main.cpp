#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <functional>
#include <utility>
#include <algorithm>
#include <filesystem>
#include <cctype>

#define MAX_ITEMS 1000 //max number of data items to produce

std::string write_to_file = "./results.csv"; //these are the defaults
std::string read_data_from_file = "./data.csv";
std::string config_file = "./config.toml";

std::vector<std::pair<long, double>> results; //data vector with the initial datas
std::vector<std::pair<long, double>> median; //the solution with time stamp and median

namespace fs = std::filesystem; //to create directories

int calculate_mean() //this function calculates mean (in case)
{
    double sum {0};
    for(int i = 0; i < results.size(); i++){
        sum += results[i].second;
    }
    sum /= results.size();
    std::cout << sum << std::endl;
    return 0;
}

int read_config() //this reads the config.toml file
{
    std::ifstream file(config_file);
    if(!file.is_open()){
        std::cerr << "could not open config file" << std::endl;
        return 1;
    }

    std::string line;
    while(getline(file, line)){
        std::stringstream ss(line);
        std::string word;
        while(ss >> word){
            if(word == "input"){
                ss >> word;
                word.erase(0, 1);
                word.erase(word.length() - 1, 1);
                read_data_from_file = word + "/data.csv";
            }
            if(word == "output"){
                ss >> word;
                word.erase(0, 1);
                word.erase(word.length() - 1, 1);
                fs::create_directories(word); //create the nested directories
                write_to_file = word + "/results.csv";
            }
        }
    }
    return 0;
}

int write_data() //write data to file
{
    std::ofstream file(write_to_file);
    if(!file.is_open()){
        std::cerr << "could not open the end file" << std::endl;
        return 1;
    }

    for(int i = 0; i < median.size(); i++){
        file << median[i].first << ";" << median[i].second << std::endl;
    }
    file.close();
    return 0;
}

int process_data() //process the data
{
    std::vector<double> calc_results; //vector to perform the calculation
    std::pair<long, double> resulting_element; //the solution

    long time;  //time stamp
    double median_number; //median for an element

    for(int i = 0; i < results.size(); i++){
        calc_results.push_back(results[i].second);
        sort(calc_results.begin(), calc_results.end());

        if(calc_results.size() % 2 != 0) //number is odd
            median_number = calc_results[calc_results.size() / 2];
        else //number is even
            median_number = (calc_results[calc_results.size() / 2] + 
        calc_results[calc_results.size() / 2 - 1 ]) / 2;

        time = results[i].first;  //the time stamp
        resulting_element.first = time;
        resulting_element.second = median_number;

        //check if the new median is different from old
        if(median.size() == 0) //ignore if its the first element
            median.push_back(resulting_element);
        else if(resulting_element.second != median[median.size() - 1].second)
            median.push_back(resulting_element); //put it into the vector
    }
    return 0;
}

int read_from_file() //read from file
{
    std::ifstream file(read_data_from_file);
    if(!file.is_open()){
        std::cerr << "could not open input file" << std::endl;
        return 1;
    }
    std::string line;
    std::vector<std::vector<std::string>> data;
    while(getline(file, line)){
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> row;
        while(getline(ss, field, ';')){
            row.push_back(field);
        }
        data.push_back(row);
    }
    file.close();

    int count {0};
    for(auto& row : data){
        std::pair<long, double> result;
        for(auto& field : row){
            if(count == 0 || count == 2){ //error checking
                if(!std::all_of(field.begin(), field.end(), [](unsigned char c){
                    return std::isdigit(c) || c == '.' || c == ',';
                })){
                    continue; //skip that part
                }
            }
            if(count == 0)
                result.first = (stol(field));
            if(count == 2)
                result.second = (stod(field));
            count++;
        }
        results.push_back(result);
        count = 0;
    }
    return 0;
}

int create_file() //create a file with data
{
    std::ofstream file(read_data_from_file);
    if(!file.is_open()){
        std::cerr << "could not open file" << std::endl;
        return 1;
    }
    std::random_device rd;

    auto price = bind(std::normal_distribution<double>{100, 10.0}, std::default_random_engine{rd()});
    auto quantity = bind(std::normal_distribution<double>{50, 15.0}, std::default_random_engine{rd()});
    
    for(int i = 0; i < MAX_ITEMS; i++){
        auto receive_ts = std::chrono::system_clock::now().time_since_epoch().count() / 1000;
        auto exchange_ts = receive_ts - 1000;
        std::string side = (receive_ts % 2 == 1)? "bid" : "ask";

        file << receive_ts << ";" << exchange_ts << ";" << 
        price() << ";" << quantity() << ";" << side << std::endl;
    }
    file.close();
    return 0;
}

int main(int argc, char* argv[])
{
    try{
        if(argc == 2){ 
            config_file = argv[1];
            read_config();  //read the config file
        }
        if(argc == 1){ 
            create_file(); //create a file with data in it
        }
        read_from_file(); //collect data into results vector
        process_data(); //process the data
        write_data(); //write the data to a file
    }
    catch(std::exception& e){
        std::cerr << "Exception has Occurred: " << e.what() << std::endl;
    }
    catch(...){
        std::cerr << "Unknown Exception has Occurred!!!!" << std::endl;
    }
    std::cout << "Data Analalyzed Successfully" << std::endl;
    //std::cout << results.size() << std::endl;
    //std::cout << median.size() << std::endl;
    //calculate_mean(); //get the mean number
    return 0;
}