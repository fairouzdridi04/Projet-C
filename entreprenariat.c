#include <stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef struct salle salle;
struct salle {
    int idsalle;  
    char nom[20];
    int capacite; 
    float tarif_horaire;
    int equipements;
    int reservation;         
};
 bool verifcapacite(salle* s,int n){
    return n <= s->capacite; 
 }
typedef struct date date;
struct date {
    int jour;
    int mois;
    int annee;
    char heure_debut[6];
    char heure_fin[6];
};
typedef struct client client ;
struct client{
    int idclient;
    char nom[20];  
    char prenom[20]; 
    char email[20];
    int numerotlf;
    char typesalle[20];
    int nbrinvite;
    date temp;
};
typedef struct reservation reservation;
struct reservation{
    client client;
    salle *salle; 
    date date;
    float tarif;
    char * statut;
    struct reservation *next;
};
reservation *tete = NULL;      
reservation *fil_tete = NULL;  
reservation *fil_queue = NULL; 
reservation *pil_tete = NULL; 

int convertir_heure_minute(char h[]){
    int H=0;
    int m=0;
    int i=0;
    while(h[i]!=':'&&h[i]!='\0'){
        H=H*10+(h[i]-'0');
        i++;
    }
    i++; 
    while(h[i]!='\0'){
        m=m*10+(h[i]-'0');
        i++;
    }
    return H*60+m;
}
int reservation_chevauche(int id_salle,date d) {
    reservation *p=tete;
    while (p) {
        if (p->salle->idsalle == id_salle &&
            p->date.jour == d.jour &&
            p->date.mois == d.mois &&
            p->date.annee == d.annee) {
            if (!(convertir_heure_minute(d.heure_fin) <= convertir_heure_minute(p->date.heure_debut) || convertir_heure_minute(d.heure_debut) >= convertir_heure_minute(p->date.heure_fin))) {
                return 1;
            }
        }
        p = p->next;
    }
    return 0; 
}
int reservconfirmation(date d,salle *s,int nbrinvite){
    return verifcapacite(s,nbrinvite)&&(!(reservation_chevauche(s->idsalle ,d)));
}
void ajouter_reservation(client c, salle *s, date d, float tarif) {
    reservation *r = malloc(sizeof(reservation));
    r->client = c;
    r->salle = s;
    r->date = d;
    r->tarif = tarif;
    r->next = NULL;
    if (reservconfirmation(d, s, c.nbrinvite)) {
        r->statut = "CONFIRMEE";
        r->next = tete;
        tete = r;
        printf("Reservation confirmee pour %s.\n", c.nom);
        generer_facture(r);
    } else {
        r->statut = "EN_ATTENTE";
        if (fil_tete == NULL) {
            fil_tete = fil_queue = r;
        } else {
            fil_queue->next = r;
            fil_queue = r;
        }
        printf("Salle occupee. Reservation mise en attente pour %s.\n", c.nom);
    }
}
void annuler_reservation(int idclient){
    if (tete == NULL) {
        printf("Aucune reservation dans la liste.\n");
        return;
    }
    reservation *p = tete;
    reservation *prec = NULL;
    while (p && p->client.idclient != idclient) {
        prec = p;
        p = p->next;
    }
    if (p == NULL) {
        printf("Aucune reservation trouvee pour le client %d.\n", idclient);
        return;
    }
    if (prec == NULL){
        tete = p->next;
    }
    else{
        prec->next = p->next;
    }
    if (p->salle) {
        p->salle->reservation = 0;
    }
    p->statut = "ANNULEE";
    p->next = pil_tete;
    pil_tete = p;
    printf("Reservation du client %d ANNULEE et ajoutee a l'historique.\n", idclient);
}
float calcul_total_simple(reservation *nouv) {
    int debut = convertir_heure_minute(nouv->client.temp.heure_debut);
    int fin = convertir_heure_minute(nouv->client.temp.heure_fin);
    int duree = fin - debut;
    int duree_h = duree / 60;
    int duree_m = duree % 60;
    float tarif = (nouv->salle->tarif_horaire * duree_h)
                + ((nouv->salle->tarif_horaire / 60.0) * duree_m);
    printf("Entrer le niveau d'equipement(1:standard ou 2:premium) :");
    scanf("%d", &nouv->salle->equipements);
    if (nouv->salle->equipements == 1)
        tarif += 10;
    else if (nouv->salle->equipements == 2)
        tarif += 20;
    return tarif;
}
void generer_facture(reservation *r) {
    if (!r || !r->salle){
        return;
    }
    char filename[50];
    sprintf(filename, "facture_%d.txt", r->client.idclient);
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Erreur lors de l'ouverture du fichier %s\n", filename);
        return;
    }
    fprintf(f, "  -FACTURE-   \n");
    fprintf(f, "Client : %s  %s\n", r->client.nom, r->client.prenom);
    fprintf(f, "Salle : %s\n", r->salle->nom);
    fprintf(f, "Date : %02d/%02d/%04d\n", r->date.jour, r->date.mois, r->date.annee);
    fprintf(f, "Montant total : %.2f DT\n", r->tarif);
    fclose(f);
    printf("Facture generee : %s\n", filename);
}
typedef struct noeudStat {
    int idsalle;
    float chiffre_affaire;
    int nb_reservations;
    struct noeudStat *gauche;
    struct noeudStat *droite;
} noeudStat;
noeudStat *racineStat = NULL;
noeudStat* creerNoeud(int id, float ca) {
    noeudStat *n = malloc(sizeof(noeudStat));
    n->idsalle = id;
    n->chiffre_affaire = ca;
    n->nb_reservations = 1;
    n->gauche = n->droite = NULL;
    return n;
}
noeudStat* insererStat(noeudStat* r, int idsalle, float tarif) {
    if (!r) {
        r = malloc(sizeof(noeudStat));
        r->idsalle = idsalle;
        r->chiffre_affaire = tarif;
        r->nb_reservations = 1;
        r->gauche = r->droite = NULL;
        return r;
    }
    if (idsalle < r->idsalle)
        r->gauche = insererStat(r->gauche, idsalle, tarif);

    else if (idsalle > r->idsalle)
        r->droite = insererStat(r->droite, idsalle, tarif);

    else {
        r->chiffre_affaire += tarif;
        r->nb_reservations++;
    }
    return r;
}
float total_CA = 0;
void calculer_chiffre_affaire() {
    reservation *p = tete;
    total_CA = 0;
    while (p) {
        racineStat = insererStat(racineStat, p->salle->idsalle, p->tarif);
        total_CA += p->tarif;
        p = p->next;
    }
}
void afficher_total_CA() {
    printf("\n-----------------------------------\n");
    printf("Chiffre d'affaire TOTAL : %.2f DT\n", total_CA);
    printf("-----------------------------------\n");
}
void afficher_stats_inorder(noeudStat *r) {
    if (!r) return;
    afficher_stats_inorder(r->gauche);
    printf("Salle %d : CA = %.2f DT | %d reservations\n",
           r->idsalle, r->chiffre_affaire, r->nb_reservations);
    afficher_stats_inorder(r->droite);
}
int compteur_mois[13] = {0};  
void compter_reservations_par_mois() {
    reservation *p = tete;
    while (p) {
        if (strcmp(p->statut, "CONFIRMEE") == 0){
            compteur_mois[p->date.mois]++;
        }
        p = p->next;
    }
}
void afficher_reservations_par_mois() {
    for (int m = 1 ; m <= 12 ; m++) {
        printf("Mois %d : %d reservations\n", m, compteur_mois[m]);
    }
}
void afficher_salles_populaires(noeudStat *r) {
    if (!r) return;
    afficher_salles_populaires(r->droite);
    printf("Salle %d -> %d reservations\n", r->idsalle, r->nb_reservations);
    afficher_salles_populaires(r->gauche);
}
void sauvegarder_reservations() {
    FILE *f = fopen("reservations.txt", "w");
    if (!f) { printf("Erreur fichier\n"); return; }
    reservation *p = tete;
    while (p) {
        fprintf(f,"%d %d %d %d %s %s %.2f %d\n",
            p->client.idclient,
            p->salle->idsalle,
            p->date.jour,
            p->date.mois,
            p->date.heure_debut,
            p->date.heure_fin,
            p->tarif,
            p->client.nbrinvite
        );
        p = p->next;
    }
    fclose(f);
    printf("Reservations sauvegardees.\n");
}
void charger_reservations(salle T[], int nbsalles) {
    FILE *f = fopen("reservations.txt", "r");
    if (!f) return;
    while (!feof(f)) {
        reservation *r = malloc(sizeof(reservation));
        int idsalle;
        fscanf(f,"%d %d %d %d %s %s %f %d",
            &r->client.idclient,
            &idsalle,
            &r->date.jour,
            &r->date.mois,
            r->date.heure_debut,
            r->date.heure_fin,
            &r->tarif,
            &r->client.nbrinvite
        );
        for (int i=0 ; i<nbsalles ; i++)
            if (T[i].idsalle == idsalle)
                r->salle = &T[i];
        r->next = tete;
        tete = r;
    }
    fclose(f);
    printf("Reservations chargees.\n");
}
bool heure_valide(char h[]) {
    if (strlen(h) != 5) return false;
    if (h[2] != ':') return false;

    int HH = (h[0]-'0')*10 + (h[1]-'0');
    int MM = (h[3]-'0')*10 + (h[4]-'0');

    return (HH >= 0 && HH < 24 && MM >= 0 && MM < 60);
}

bool numero_valide(int num) {
    return num >= 10000000 && num <= 99999999; 
}
int main() {
    int choix_principal;
    int nbsalles = 0;
    salle *T = NULL;
    int salles_creees = 0;
    while (1) {
        printf("  -MENU PRINCIPAL-  \n");
        printf("1. Mode ADMIN\n");
        printf("2. Mode UTILISATEUR\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix_principal);
        if (choix_principal == 0) {
            printf("\n Au revoir !\n");
            break;
        }
        else if (choix_principal == 1) {
            if (!salles_creees) {
                printf("\n   -ADMIN : Creation des salles-   \n");
                printf("Entrer le nombre de salles : ");
                scanf("%d", &nbsalles);
                T = malloc(nbsalles * sizeof(salle));
                for (int i = 0; i < nbsalles; i++) {
                    printf("\n - Salle %d -\n", i + 1);
                    printf("ID salle : "); scanf("%d", &T[i].idsalle);
                    printf("Nom : "); scanf("%s", T[i].nom);
                    printf("Capacite : "); scanf("%d", &T[i].capacite);
                    printf("Tarif horaire : "); scanf("%f", &T[i].tarif_horaire);
                    T[i].reservation = 0;
                    T[i].equipements = 0;
                }
                salles_creees = 1;
                printf("\n  -salles enregistrees-   \n");
            }
            int choixAdmin;
            do {
                printf("\n    -interface ADMIN-    \n");
                printf("1. Afficher statistiques\n");
                printf("2. Afficher file d'attente\n");
                printf("3. Afficher pile d'annulations\n");
                printf("4. Sauvegarder les reservations\n");
                printf("5. Charger les reservations\n");
                printf("0. Retour\n");
                printf("Votre choix : ");
                scanf("%d", &choixAdmin);
                if (choixAdmin == 1) {
                    calculer_chiffre_affaire();
                    afficher_stats_inorder(racineStat);
                    afficher_total_CA();
                    compter_reservations_par_mois();
                    afficher_reservations_par_mois();
                    printf("\n Salles les plus populaires :\n");
                    afficher_salles_populaires(racineStat);
                }
                else if (choixAdmin == 2) {
                    printf("\n   -LISTE D'ATTENTE-   \n");
                    reservation *p = fil_tete;
                    if (!p) printf("Aucune reservation en attente.\n");
                    while (p) {
                        printf("Client %d - Salle %d - STATUT : %s\n",
                               p->client.idclient, p->salle->idsalle, p->statut);
                        p = p->next;
                    }
                }
                else if (choixAdmin == 3) {
                    printf("\n   -LISTE DES ANNULATIONS-   \n");
                    reservation *p = pil_tete;
                    if (!p) printf("Aucune reservation annulee.\n");
                    while (p) {
                        printf("Client %d - Salle %d - STATUT : %s\n",
                               p->client.idclient, p->salle->idsalle, p->statut);
                        p = p->next;
                    }
                }
                else if (choixAdmin == 4) {
                    sauvegarder_reservations();
                }
                else if (choixAdmin == 5) {
                    charger_reservations(T, nbsalles);
                }
            } while (choixAdmin != 0);
        }
        else if (choix_principal == 2) {
            if (!salles_creees) {
                printf("Veuillez d'abord entrer dans l'espace administrateur pour creer les salles.\n");
                continue;
            }
            int choixUser;
            do {
                printf("\n   -interface UTILISATEUR-   \n");
                printf("1. Faire une reservation\n");
                printf("2. Annuler une reservation\n");
                printf("0. Retour\n");
                printf("Votre choix : ");
                scanf("%d", &choixUser);
                if (choixUser == 0) break;
                else if (choixUser == 1) {
                    client c;
                    date d;
                    printf("\n   -NOUVELLE RESERVATION-   \n");
                    printf("ID client : "); scanf("%d", &c.idclient);
                    printf("Nom : "); scanf("%s", c.nom);
                    printf("Prenom : "); scanf("%s", c.prenom);
                    printf("Email : "); scanf("%s", c.email);
                    do {
                        printf("Numero telephone : ");
                        scanf("%d", &c.numerotlf);
                    } while (!numero_valide(c.numerotlf));
                    printf("Type de salle demandee : "); scanf("%s", c.typesalle);
                    do {
                        printf("Nombre d'invites : ");
                        scanf("%d", &c.nbrinvite);
                    } while (c.nbrinvite <= 0);
                    printf("Jour : "); scanf("%d", &d.jour);
                    printf("Mois : "); scanf("%d", &d.mois);
                    printf("Annee : "); scanf("%d", &d.annee);
                    do {
                        printf("Heure debut (HH:MM) : ");
                        scanf("%s", d.heure_debut);
                    } while (!heure_valide(d.heure_debut));
                    do {
                        printf("Heure fin (HH:MM) : ");
                        scanf("%s", d.heure_fin);
                    } while (!heure_valide(d.heure_fin));
                    salle *s = NULL;
                    int peut_confirmer = 0;
                    for (int i = 0; i < nbsalles; i++) {
                        if (reservconfirmation(d, &T[i], c.nbrinvite)) {
                            s = &T[i];
                            peut_confirmer = 1;
                            break;
                        }
                    }
                    if (!peut_confirmer) {
                        for (int i = 0; i < nbsalles; i++) {
                            if (verifcapacite(&T[i], c.nbrinvite)) {
                                s = &T[i];
                                break;
                            }
                        }
                    }
                    if (!s) {
                        printf("Aucune salle disponible pour %d invites ! \n", c.nbrinvite);
                        continue;
                    }
                    reservation *r = malloc(sizeof(reservation));
                    r->client = c;
                    r->salle = s;
                    r->date = d;
                    r->client.temp = d;
                    r->tarif = calcul_total_simple(r);
                    ajouter_reservation(c, s, d, r->tarif); 
                }
                else if (choixUser == 2) {
                    int id;
                    printf("Entrer ID client a annuler : ");
                    scanf("%d", &id);
                    annuler_reservation(id);
                }
            } while (1);
        }
    }
    return 0;

}