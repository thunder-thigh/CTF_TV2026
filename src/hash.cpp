#include <iostream>
#include <iomanip>
#include <openssl/evp.h>

std::string hash_password(std::string password){
	//printf("Buffer Size: %d\n",EVP_MAX_BLOCK_LENGTH);
	unsigned char hash[EVP_MAX_BLOCK_LENGTH];
	size_t hash_len=0;
	if (!EVP_Q_digest(nullptr, "SHA256", nullptr, password.data(), password.size(), hash, &hash_len)){
		return "Error, hashing password failed";
	}
	std::stringstream ss;
	for (size_t i = 0; i < hash_len; i++){
	ss<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(hash[i]);
	}
	std::string password_hashed = ss.str();
	return password_hashed;
}
