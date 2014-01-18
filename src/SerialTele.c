/*
 * SerialTele.c
 *
 *  Created on: Nov 14, 2012
 *      Author: eric
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include "SerialDefine.h"

void TraiteChaine(char * ligne, Pulsadis * pPulsadis);

char EncoreEtToujours;

int OpenSerialTeleinfo(char * sPort)
{
    struct termios options;

    //Opening communication port
    int fd = open(sPort, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1)
    {
        perror("Unable to open serial port");
    }
    tcgetattr(fd, &options);
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;

    options.c_cflag &= ~CRTSCTS;
    cfsetispeed(&options,B1200);
    cfsetospeed(&options,B1200);
    options.c_iflag &= ~(IXON|IXOFF|IXANY);
    options.c_cc[VMIN] = 1;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag |= (INPCK | ISTRIP);
    tcsetattr(fd,TCSANOW,&options);
    fcntl(fd, F_SETFL, 0);
    return fd;
}

void CloseSerialTeleinfo(int fd)
{
    close(fd);
}

void GestionHandler(int s)
{
    printf("Reception de l'action %d\n",s);
    EncoreEtToujours = 0;
}

int main(int argc, char * argv[])
{
    char SerialPort[] = "/dev/ttyAMA0";
    char travail;
    int nombre;
    int index = 0;
    int fd;
    char Ligne[30];
    int mem;
    static Pulsadis * pPulsadis;
    struct sigaction sigHandler;

    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
            exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
            exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }



    /* Change the current working directory */
    if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    EncoreEtToujours = 1;
    sigHandler.sa_handler = GestionHandler;
    sigHandler.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM,&sigHandler,NULL);
    fd = OpenSerialTeleinfo(SerialPort);
    if(fd < 0)
    {
        printf("Erreur d'ouverture du port serie\n");
        return 1;
    }
    mem = shm_open("SerialInfo.Pulsadis",O_CREAT | O_RDWR, 0666);
    if(mem < 0)
    {
        mem = shm_open("SerialInfo.Pulsadis",O_RDWR, 0666);
        if(mem < 0)
        {
            printf("Erreur d'allocation memoire\n");
            CloseSerialTeleinfo(fd);
            return 2;
        }
    }
    ftruncate(mem,sizeof(Pulsadis));
    pPulsadis = mmap(0,sizeof(Pulsadis),PROT_READ | PROT_WRITE,MAP_SHARED,mem,0);
    if(pPulsadis == MAP_FAILED)
    {
        printf("Erreur de mmap \n");
        return 3;
    }
    memset(pPulsadis,0,sizeof(Pulsadis));
    printf("Opening serial com : %s : %d\n",SerialPort,fd);
    while(EncoreEtToujours)
    {
        memset(Ligne,0,sizeof(Ligne));
        //sequence = 0;
        index = 0;
        //Attente de synchronisation
        printf("Waiting sync\n");
        while(EncoreEtToujours)
        {
            nombre = read(fd,&travail,sizeof(char));
            if(nombre > 0  && travail == 0x02) break;
        }
        printf("End Sync\n");
        while(EncoreEtToujours)
        {
            nombre = read(fd,&travail,sizeof(char));
            if(nombre > 0)
            {
                switch(travail)
                {
                    case 0x03:
                    break;
                    case 0x0A:
                        index = 0;
                        TraiteChaine(Ligne,pPulsadis);
                    break;
                    default:
                        Ligne[index++] = travail;
                        Ligne[index] = 0x00;
                break;

                }
            }
        }
    }
    printf("Closing serial com\n");
    CloseSerialTeleinfo(fd);
    printf("Closing shared Memory\n");
    close(mem);
    return 0;
}

void TraiteChaine(char * ligne, Pulsadis * pPulsadis)
{
    char * MotCle;
    char * Value;
    MotCle = strtok(ligne," ");
    Value = strtok(NULL," ");
    if(MotCle != NULL && Value != NULL)
    {
        if(strcmp(MotCle,"ADCO") == 0)
        {
            strcpy(pPulsadis->Adco,Value);
        }
        else if(strcmp(MotCle,"OPTARIF") == 0 )
        {
            if(strcmp(Value,"BASE") == 0 )
            {
                pPulsadis->OptTarifaire = Base;
            }
            else if(strcmp(Value,"HC..") == 0 )
            {
                pPulsadis->OptTarifaire = Creuse;
            }
            else if(strcmp(Value,"EJP.") == 0 )
            {
                pPulsadis->OptTarifaire = Ejp;
            }
            else if(strcmp(Value,"BBR(") == 0 )
            {
                pPulsadis->OptTarifaire = Tempo;
            }
        }
        else if(strcmp(MotCle,"ISOUSC") == 0)
        {
            pPulsadis->IntensiteSouscrite = atoi(Value);
        }
        else if(strcmp(MotCle,"BASE") == 0)
        {
            pPulsadis->IndexBase = atol(Value);
        }
        else if(strcmp(MotCle,"HCHC") == 0)
        {
            pPulsadis->IndexHeureCreuse = atol(Value);
        }
        else if(strcmp(MotCle,"HCHP") == 0)
        {
            pPulsadis->IndexHeurePleine = atol(Value);
        }
        else if(strcmp(MotCle,"EJPHN") == 0)
        {
            pPulsadis->IndexEjpCreuse = atol(Value);
        }
        else if(strcmp(MotCle,"EJPHM") == 0)
        {
            pPulsadis->IndexEjpPleine = atol(Value);
        }
        else if(strcmp(MotCle,"GAZ") == 0)
        {
            pPulsadis->IndexGaz = atol(Value);
        }
        else if(strcmp(MotCle,"AUTRE") == 0)
        {
            pPulsadis->IndexAutre = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHCJB") == 0)
        {
            pPulsadis->IndexBleuCreuse = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHPJB") == 0)
        {
            pPulsadis->IndexBleuPleine = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHCJW") == 0)
        {
            pPulsadis->IndexBlancCreuse = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHPJW") == 0)
        {
            pPulsadis->IndexBlancPleine = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHCJR") == 0)
        {
            pPulsadis->IndexRougeCreuse = atol(Value);
        }
        else if(strcmp(MotCle,"BBRHPJR") == 0)
        {
            pPulsadis->IndexRougePleine = atol(Value);
        }
        else if(strcmp(MotCle,"PEJP") == 0)
        {
            pPulsadis->PreavisEjp = atoi(Value);
        }
        else if(strcmp(MotCle,"IINST") == 0)
        {
            pPulsadis->Instantanee = atol(Value);
        }
        else if(strcmp(MotCle,"ADPS") == 0)
        {
            pPulsadis->Depassement = atoi(Value);
        }
        else if(strcmp(MotCle,"IMAX") == 0)
        {
            pPulsadis->IntensiteMaximale = atoi(Value);
        }
        else if(strcmp(MotCle,"MOTDETAT") == 0)
        {
            strcpy(pPulsadis->MotDetat,Value);
        }
        else if(strcmp(MotCle,"PTEC") == 0)
        {
            if(strcmp(Value,"TH..") == 0)
            {
                pPulsadis->periode = ToutesHeures;
            }
            else if(strcmp(Value,"HC..") == 0)
            {
                pPulsadis->periode = HeureCreuse;
            }
            else if(strcmp(Value,"HP..") == 0)
            {
                pPulsadis->periode = HeurePleine;
            }
            else if(strcmp(Value,"HN..") == 0)
            {
                pPulsadis->periode = HeureNormale;
            }
            else if(strcmp(Value,"PM..") == 0)
            {
                pPulsadis->periode = JourDePointeMobile;
            }
            else if(strcmp(Value,"HCJB") == 0)
            {
                pPulsadis->periode = HeureCreuseJourBleu;
            }
            else if(strcmp(Value,"HPJB") == 0)
            {
                pPulsadis->periode = HeurePleineJourBleu;
            }
            else if(strcmp(Value,"HCJW") == 0)
            {
                pPulsadis->periode = HeureCreuseJourBlanc;
            }
            else if(strcmp(Value,"HPJW") == 0)
            {
                pPulsadis->periode = HeurePleineJourBlanc;
            }
            else if(strcmp(Value,"HCJR") == 0)
            {
                pPulsadis->periode = HeureCreuseJourRouge;
            }
            else if(strcmp(Value,"HPJR") == 0)
            {
                pPulsadis->periode = HeurePleineJourRouge;
            }
        }
        else if(strcmp(MotCle,"DEMAIN") == 0)
        {
            if(strcmp(Value,"----") == 0)
            {
                pPulsadis->demain = Inconnu;
            }
            else if(strcmp(Value,"BLEU") == 0)
            {
                pPulsadis->demain = Bleu;
            }
            else if(strcmp(Value,"BLAN") == 0)
            {
                pPulsadis->demain = Blanc;
            }
            else if(strcmp(Value,"ROUG") == 0)
            {
                pPulsadis->demain = Rouge;
            }
        }
        else if(strcmp(MotCle,"PAPP") == 0)
        {
            pPulsadis->PuissanceApparente = atoi(Value);
        }

    }
}


