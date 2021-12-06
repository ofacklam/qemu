#include <algorithm>
#include <iostream>
#include <string>

int main() {
	
	std::string sentence;
	std::getline(std::cin, sentence, '\n');
	for (int i = 0; i < sentence.size()/2; i++) {
		std::swap(sentence[i], sentence[sentence.size()-1-i]);
	}
	std::cout << sentence << std::endl;
    
	return 0;
}

