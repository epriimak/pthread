#include <iostream>
#include <cstdlib>


#include "SetOS.h"
#include "SetLS.h"

#define NUM_EL_IN_SET 10000
#define NUM_THR 5

typedef void* (*func)(void* args);

SetOS<int>* pSet1 = NULL;
SetLS<int>* pSet2 = NULL;

void* writeOS(void* args){
	for (int i = 0; i < NUM_EL_IN_SET; i++){
		int num = i + 1;
		pSet1->add(num);
	}
 	pthread_exit(0);
}

void* writeLS(void* args){
	for (int i = 0; i < NUM_EL_IN_SET; i++){
		int num = i + 1;
		pSet2->add(num);
	}
 	pthread_exit(0);
}


void* readOS(void* args){
	int* count = reinterpret_cast<int*>(args);
	*count = 0;
	for(int i = 0; i < NUM_EL_IN_SET; i++){
		int num = i + 1;
		if (pSet1->contains(num) && pSet1->remove(num)){
			++(*count);
		}
	}
	pthread_exit(0);
}


void* readLS(void* args){
	int* count = reinterpret_cast<int*>(args);
	*count = 0;
	for(int i = 0; i < NUM_EL_IN_SET; i++){
		int num = i + 1;
		if (pSet2->contains(num) && pSet2->remove(num)){
			++(*count);		
		}
	}
	pthread_exit(0);
}

bool many2one(func pWriteFunc, func pReadFunc){
	pSet1 = new SetOS<int>();
	pSet2 = new SetLS<int>();
	
	pthread_t thrR;
	pthread_t thrW[NUM_THR];
	int count = 0;

	for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrW[i], NULL, pWriteFunc, NULL) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

    for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrW[i], NULL);	
    }
    
    if (pthread_create(&thrR, NULL, pReadFunc, &count) != 0)
		syslog(LOG_INFO, "Error creating thread");
  	
  	pthread_join(thrR, NULL);
	
    delete pSet1;
    delete pSet2;
    
    return (count == NUM_EL_IN_SET);
}

bool one2many(func pWriteFunc, func pReadFunc){
	pSet1 = new SetOS<int>();
	pSet2 = new SetLS<int>();

	pthread_t thrW;
	pthread_t thrR[NUM_THR];
	int count[NUM_THR];

	if (pthread_create(&thrW, NULL, pWriteFunc, NULL) != 0)
    	syslog(LOG_INFO, "Error creating thread");
    pthread_join(thrW, NULL);

    for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrR[i], NULL, pReadFunc, &count[i]) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

    for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrR[i], NULL);	
    }

	int sum = 0;
    for(int i = 0; i < NUM_THR; i++){
    	sum += count[i];
    }
    
    delete pSet1;
    delete pSet2;
    
    return (sum == NUM_EL_IN_SET);
}

bool many2many(func pWriteFunc, func pReadFunc){
	pSet1 = new SetOS<int>();
	pSet2 = new SetLS<int>();

	pthread_t thrR[NUM_THR];
	pthread_t thrW[NUM_THR];
	int count[NUM_THR];

	for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrW[i], NULL, pWriteFunc, NULL) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

    for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrW[i], NULL);	
    }
    
    for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrR[i], NULL, pReadFunc, &count[i]) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

    for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrR[i], NULL);	
    }

	int sum = 0;
    for(int i = 0; i < NUM_THR; i++){
    	sum += count[i];
    }

    delete pSet1;
    delete pSet2;   
    
    return (sum == NUM_EL_IN_SET);
}

bool many2manyTogether(func pWriteFunc, func pReadFunc){
	pSet1 = new SetOS<int>();
	pSet2 = new SetLS<int>();

	pthread_t thrR[NUM_THR];
	pthread_t thrW[NUM_THR];
	pthread_t thr;
	int count[NUM_THR];


	for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrW[i], NULL, pWriteFunc, NULL) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

  	for(int i = 0; i < NUM_THR; i++){
		if (pthread_create(&thrR[i], NULL, pReadFunc, &count[i]) != 0)
    		syslog(LOG_INFO, "Error creating thread");
  	}

	for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrW[i], NULL);	
    }

    for(int i = 0; i < NUM_THR; i++){
    	pthread_join(thrR[i], NULL);	
    }


    int restCount = 0;
    if (pthread_create(&thr, NULL, pReadFunc, &restCount) != 0)
    	syslog(LOG_INFO, "Error creating thread");
    pthread_join(thr, NULL);

	int sum = 0;
    for(int i = 0; i < NUM_THR; i++){
    	sum += count[i];
    }

    delete pSet1;
    delete pSet2;   
    
    return ((sum + restCount) == NUM_EL_IN_SET);
}

void printRes(bool res){
	res ? std::cout << "OK" : std::cout << "WRONG";
	std::cout << std:: endl;
}

int main()
{
    //test for OS
   	std::cout << "OS: many2one: ";
    printRes(many2one(&writeOS, &readOS));

    std::cout << "OS: one2many: ";
    printRes(one2many(&writeOS, &readOS));

    std::cout << "OS: many2many: ";
    printRes(many2many(&writeOS, &readOS));

    std::cout << "OS: many2manyTogether: ";
    printRes(many2manyTogether(&writeOS, &readOS));
    
    //test for LS
   	std::cout << "LS: many2one: ";
    printRes(many2one(&writeLS, &readLS));

    std::cout << "LS: one2many: ";
    printRes(one2many(&writeLS, &readLS));

	std::cout << "LS: many2many: ";
    printRes(many2many(&writeLS, &readLS));

    std::cout << "LS: many2manyTogether: ";
    printRes(many2manyTogether(&writeLS, &readLS));
    
    return 0;
}