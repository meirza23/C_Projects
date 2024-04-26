/**
* zombie.c zombie slayer simulator:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h> 

#define Max_Doorman 10
#define Max_Slayer 3
#define Max_Zombie 100

int getKilledCount();

int ZombiSayaci=0;
int gameover=0;
int KillCount=0;

/* Keeps track of number of zombies entered.*/
void zombieEntered(){
    if(ZombiSayaci < Max_Zombie)
        ZombiSayaci++;
}

/* Keeps track of number of zombies killed.*/
void zombieKilled(){
    if(ZombiSayaci > 0){
        ZombiSayaci--;
        KillCount++;
    }
}

/* Returns true if number of zombies in the room are 
greater than or equal to 100.*/
int tooManyZombiesInTheRoom(){
    if(ZombiSayaci >= Max_Zombie)
        return 1;
    else
        return 0;
}


/*Returns true if more than 100 zombies have been killed.*/
int killed100Zombies(){
    int sonuc;
    if(getKilledCount() >= 100)
        sonuc = 1;
    else
        sonuc = 0;
    return sonuc;
}


/* Returns true if there is at least one zombies in the room.*/
int zombiesExist(){
    int sonuc;
    if(ZombiSayaci>0)
        sonuc=1;
    return sonuc;
}
/*Returns the number of zombies killed.*/
int getKilledCount(){
    return KillCount;
}

/* Returns the number of zombies in the room.*/
int getInTheRoomCount(){
    int sonuc;
    sonuc = ZombiSayaci;
    return sonuc;
}

/*doorman thread*/
void *doorMan(void *p){
    while (!gameover)
    {
        if (rand()%2 == 0){
            zombieEntered();
        }
        usleep(2000);
        if (tooManyZombiesInTheRoom()) {
            printf("Odada cok fazla zombi var,oyun biter.\n");
            gameover = 1;
        }
    }
    pthread_exit(NULL); 
}

/*slayer thread*/
void *slayer(void *p){
    while (!gameover) {
        if (zombiesExist()) {
            zombieKilled();
            if (killed100Zombies()) {
                printf("Tebrikler 100 zombi öldürdünüz.Oyun Biter!!\n");
                gameover = 1;
            }
        }
        usleep(2000); // Sleep for 2 ms
    }
    pthread_exit(NULL); 
}
/*simulator main thread*/
int main(int argc, char **argv) {
    int i;
    pthread_t doorManThreads[Max_Doorman];
    pthread_t slayerThreads[Max_Slayer];

    for (i = 0; i < Max_Doorman; i++) {
        pthread_create(&doorManThreads[i], NULL, doorMan, NULL);
    }

    for (i = 0; i < Max_Slayer; i++) {
        pthread_create(&slayerThreads[i], NULL, slayer, NULL);
    }

    for (i = 0; i < Max_Doorman; i++) {
        pthread_join(doorManThreads[i], NULL);
    }

    for (i = 0; i < Max_Slayer; i++) {
        pthread_join(slayerThreads[i], NULL);
    }

    printf("Oldurulen zombi sayisi: %d\n", getKilledCount());
    return 0;
}
