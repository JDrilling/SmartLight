#include <iostream>
#include "NRF.h"
#include <string>

int main(){
  std::string input;
  NRF nrf;
  
  nrf.init();

  do{
    std::cout << "Enter Command:   ";
    std::cin >> input;
    std::cout << std::endl;

    nrf.sendMessage(input, 0);

  }while(input != "quit");
  return 0;
}
