/*
 * SerialDefine.h
 *
 *  Created on: Nov 23, 2012
 *      Author: eric
 */

#ifndef SERIALDEFINE_H_
#define SERIALDEFINE_H_


typedef enum
{
    Base,
    Creuse,
    Ejp,
    Tempo
}OptionTarifaire;

typedef enum
{
    ToutesHeures,
    HeureCreuse,
    HeurePleine,
    HeureNormale,
    JourDePointeMobile,
    HeureCreuseJourBleu,
    HeurePleineJourBleu,
    HeureCreuseJourBlanc,
    HeurePleineJourBlanc,
    HeureCreuseJourRouge,
    HeurePleineJourRouge
}PeriodeTarifaire;

typedef enum
{
    Inconnu,
    Bleu,
    Blanc,
    Rouge
}CouleurDemain;

typedef struct _defPulsadis
{
    char Adco[13];
    OptionTarifaire OptTarifaire;
    int  IntensiteSouscrite;
    long IndexBase;
    long IndexHeureCreuse;
    long IndexHeurePleine;
    long IndexEjpCreuse;
    long IndexEjpPleine;
    long IndexGaz;
    long IndexAutre;
    long IndexBleuCreuse;
    long IndexBleuPleine;
    long IndexBlancCreuse;
    long IndexBlancPleine;
    long IndexRougeCreuse;
    long IndexRougePleine;
    int PreavisEjp;
    PeriodeTarifaire periode;
    CouleurDemain demain;
    int Instantanee;
    int Depassement;
    int IntensiteMaximale;
    int PuissanceApparente;
    char MotDetat[7];
}Pulsadis;


#endif /* SERIALDEFINE_H_ */
