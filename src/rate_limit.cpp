#include <iostream>
#include <string>
#include <unordered_map>
#include <ctime>

int rate_limit_interval=-1;		//Default = 1, Disable with -1 

bool check_rate_limit(std::unordered_map<std::string, std::time_t>& ip_table, std::string IP){
	try {
		std::time_t current_time=std::time(nullptr);
		int interval=current_time-ip_table.at(IP);
		if (interval>rate_limit_interval) {
			//std::cout<<"Current time: "<<current_time<<std::endl;
			//std::cout<<"Interval: "<<interval<<std::endl;
			ip_table[IP]=std::time(nullptr);
			return true;
		}else{
			ip_table[IP]=std::time(nullptr);
			return false;
		}
	} catch(const std::out_of_range& oor){
		//std::cerr<<"Error, IP not found in ip_table\n"<<std::endl;
		ip_table[IP]=std::time(nullptr);
		//std::cout<<"IP: "<<IP<<"\n";
		//std::cerr<<"IP logged into table\n"<<std::endl;
		return true;
	}
}
