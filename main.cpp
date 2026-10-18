#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <random>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <limits>

using namespace std;
using namespace chrono;

// =====================================================
// 1. PARAMETRES
// =====================================================

const uint64_t NB_MOTS_TEST = 10000000;

const string FICHIER_SOURCE = "mots.txt";
const string FICHIER_HUFFMAN = "mots.huff";
const string FICHIER_FIND = "resultats_find.txt";
const string FICHIER_MANUEL = "resultats_manuel.txt";

// =====================================================
// 2. GENERATION DU FICHIER DE MOTS
// =====================================================

void genererFichier(uint64_t nombreMots)
{
    ofstream fichier(FICHIER_SOURCE.c_str());

    if (!fichier)
    {
        cerr << "Erreur : impossible de creer mots.txt" << endl;
        return;
    }

    random_device rd;
    mt19937 generateur(rd());

    // Frequences approximatives pour favoriser certaines lettres.
    vector<double> poids = {
        80, 15, 30, 35, 120, 20, 18, 45, 70, 4, 8, 40, 25,
        68, 75, 20, 3, 60, 65, 90, 30, 12, 15, 5, 15, 1
    };

    discrete_distribution<int> choixLettre(
        poids.begin(), poids.end()
    );

    uniform_int_distribution<int> longueur(5, 15);

    vector<string> exemples = {
        "compteur",
        "programmation",
        "informatique",
        "ordinateur",
        "algorithme"
    };

    cout << "\n[1/5] Generation du fichier..." << endl;

    for (uint64_t i = 0; i < nombreMots; i++)
    {
        string mot;

        if (i < exemples.size())
        {
            mot = exemples[i];
        }
        else
        {
            int taille = longueur(generateur);

            for (int j = 0; j < taille; j++)
            {
                mot += char('a' + choixLettre(generateur));
            }
        }

        fichier << mot << '\n';

        if ((i + 1) % 1000000 == 0)
        {
            cout << "  " << i + 1 << " mots generes." << endl;
        }
    }

    fichier.close();

    cout << "Generation terminee." << endl;
}

// =====================================================
// 3. RECHERCHE AVEC FIND()
// =====================================================

bool rechercheFind(const string& mot, const string& sousChaine)
{
    return mot.find(sousChaine) != string::npos;
}

// =====================================================
// 4. RECHERCHE MANUELLE FROM SCRATCH
// =====================================================

bool fromScratch(const string& mot, const string& sousChaine)
{
    if (sousChaine.empty())
        return true;

    if (sousChaine.length() > mot.length())
        return false;

    for (size_t i = 0;
         i <= mot.length() - sousChaine.length();
         i++)
    {
        size_t j = 0;

        while (j < sousChaine.length() &&
               mot[i + j] == sousChaine[j])
        {
            j++;
        }

        if (j == sousChaine.length())
            return true;
    }

    return false;
}

// =====================================================
// 5. REMPLACEMENT AVEC FIND()
// Remplace toutes les occurrences non chevauchantes.
// =====================================================

string remplacerFind(string mot,
                     const string& sousChaine,
                     const string& remplacement)
{
    if (sousChaine.empty())
        return mot;

    size_t position = 0;

    while ((position = mot.find(sousChaine, position))
           != string::npos)
    {
        mot.replace(position, sousChaine.length(), remplacement);

        // Reprendre apres le texte insere afin de ne pas
        // rechercher a nouveau dans le remplacement lui-meme.
        position += remplacement.length();
    }

    return mot;
}

// =====================================================
// 6. REMPLACEMENT MANUEL
// Remplace toutes les occurrences non chevauchantes,
// sans utiliser find().
// =====================================================

string remplacerFromScratch(string mot,
                            const string& sousChaine,
                            const string& remplacement)
{
    if (sousChaine.empty())
        return mot;

    string resultat;
    size_t i = 0;

    while (i < mot.length())
    {
        size_t j = 0;

        // Comparaison caractere par caractere.
        while (j < sousChaine.length() &&
               i + j < mot.length() &&
               mot[i + j] == sousChaine[j])
        {
            j++;
        }

        if (j == sousChaine.length())
        {
            // Une occurrence complete a ete trouvee.
            resultat += remplacement;

            // Sauter la sous-chaine originale.
            i += sousChaine.length();
        }
        else
        {
            // Aucun motif complet a cette position :
            // recopier le caractere et avancer.
            resultat += mot[i];
            i++;
        }
    }

    return resultat;
}

// =====================================================
// 7. RECHERCHE ET REMPLACEMENT DANS LE FICHIER
// Traitement mot par mot pour limiter la memoire.
// =====================================================

bool traiterFichier(const string& sousChaine,
                    const string& remplacement,
                    bool utiliserFind,
                    uint64_t& nombreMotsTrouves,
                    double& tempsTraitement)
{
    ifstream entree(FICHIER_SOURCE.c_str());

    const string nomSortie =
        utiliserFind ? FICHIER_FIND : FICHIER_MANUEL;

    ofstream sortie(nomSortie.c_str());

    if (!entree || !sortie)
    {
        cerr << "Erreur lors de l'ouverture des fichiers." << endl;
        return false;
    }

    nombreMotsTrouves = 0;

    auto debut = steady_clock::now();

    string mot;
    uint64_t compteur = 0;

    cout << (utiliserFind
             ? "\n[2/5] Recherche et remplacement avec find()..."
             : "\n[3/5] Recherche et remplacement avec fromScratch()...")
         << endl;

    while (getline(entree, mot))
    {
        bool trouve = utiliserFind
                    ? rechercheFind(mot, sousChaine)
                    : fromScratch(mot, sousChaine);

        if (trouve)
        {
            // Compte les mots contenant la sous-chaine,
            // pas le nombre total d'occurrences.
            nombreMotsTrouves++;

            mot = utiliserFind
                ? remplacerFind(mot, sousChaine, remplacement)
                : remplacerFromScratch(mot, sousChaine, remplacement);
        }

        sortie << mot << '\n';
        compteur++;

        if (compteur % 1000000 == 0)
        {
            cout << "  " << compteur << " mots traites." << endl;
        }
    }

    auto fin = steady_clock::now();
    tempsTraitement = duration<double>(fin - debut).count();

    entree.close();
    sortie.close();

    cout << "Mots contenant la sous-chaine : "
         << nombreMotsTrouves << endl;
    cout << "Fichier resultat : " << nomSortie << endl;
    cout << "Temps recherche + remplacement + ecriture : "
         << fixed << setprecision(4)
         << tempsTraitement << " secondes." << endl;

    return true;
}

// =====================================================
// 8. STRUCTURE D'UN NOEUD HUFFMAN
// =====================================================

struct Noeud
{
    unsigned char caractere;
    uint64_t frequence;
    Noeud* gauche;
    Noeud* droite;

    Noeud(unsigned char c, uint64_t f)
        : caractere(c), frequence(f),
          gauche(NULL), droite(NULL) {}

    Noeud(Noeud* g, Noeud* d)
        : caractere(0),
          frequence(g->frequence + d->frequence),
          gauche(g), droite(d) {}

    bool estFeuille() const
    {
        return gauche == NULL && droite == NULL;
    }
};

struct ComparerNoeuds
{
    bool operator()(const Noeud* a, const Noeud* b) const
    {
        return a->frequence > b->frequence;
    }
};

void libererArbre(Noeud* noeud)
{
    if (noeud == NULL)
        return;

    libererArbre(noeud->gauche);
    libererArbre(noeud->droite);
    delete noeud;
}

Noeud* construireArbre(const array<uint64_t, 256>& frequences)
{
    priority_queue<Noeud*, vector<Noeud*>, ComparerNoeuds> file;

    for (int i = 0; i < 256; i++)
    {
        if (frequences[i] > 0)
        {
            file.push(new Noeud(
                static_cast<unsigned char>(i), frequences[i]
            ));
        }
    }

    if (file.empty())
        return NULL;

    while (file.size() > 1)
    {
        Noeud* a = file.top();
        file.pop();

        Noeud* b = file.top();
        file.pop();

        file.push(new Noeud(a, b));
    }

    return file.top();
}

void construireCodes(Noeud* noeud,
                     const string& prefixe,
                     array<string, 256>& codes)
{
    if (noeud == NULL)
        return;

    if (noeud->estFeuille())
    {
        codes[noeud->caractere] =
            prefixe.empty() ? "0" : prefixe;
        return;
    }

    construireCodes(noeud->gauche, prefixe + "0", codes);
    construireCodes(noeud->droite, prefixe + "1", codes);
}

// =====================================================
// 9. ECRITURE DES BITS DANS UN FICHIER BINAIRE
// =====================================================

class EcrivainBits
{
private:
    ofstream& fichier;
    unsigned char octet;
    int nombreBits;

public:
    explicit EcrivainBits(ofstream& f)
        : fichier(f), octet(0), nombreBits(0) {}

    void ecrireBit(int bit)
    {
        octet = static_cast<unsigned char>(
            (octet << 1) | (bit ? 1 : 0)
        );

        nombreBits++;

        if (nombreBits == 8)
        {
            fichier.put(static_cast<char>(octet));
            octet = 0;
            nombreBits = 0;
        }
    }

    void ecrireCode(const string& code)
    {
        for (size_t i = 0; i < code.length(); i++)
        {
            ecrireBit(code[i] == '1');
        }
    }

    void terminer()
    {
        if (nombreBits > 0)
        {
            octet = static_cast<unsigned char>(
                octet << (8 - nombreBits)
            );

            fichier.put(static_cast<char>(octet));
            octet = 0;
            nombreBits = 0;
        }
    }
};

// =====================================================
// 10. COMPRESSION HUFFMAN
// =====================================================

bool compresserHuffman(uint64_t& tailleOriginale,
                       uint64_t& tailleCompressee,
                       double& tempsCompression)
{
    cout << "\n[4/5] Compression Huffman..." << endl;
    cout << "  Calcul des frequences..." << endl;

    auto debut = steady_clock::now();

    array<uint64_t, 256> frequences = {};

    ifstream entree(FICHIER_SOURCE.c_str(), ios::binary);

    if (!entree)
    {
        cerr << "Erreur : impossible de lire mots.txt" << endl;
        return false;
    }

    tailleOriginale = 0;
    char c;

    while (entree.get(c))
    {
        unsigned char caractere = static_cast<unsigned char>(c);
        frequences[caractere]++;
        tailleOriginale++;
    }

    entree.close();

    cout << "  Construction de l'arbre Huffman..." << endl;

    Noeud* racine = construireArbre(frequences);

    if (racine == NULL)
    {
        cerr << "Erreur : le fichier est vide." << endl;
        return false;
    }

    array<string, 256> codes = {};
    construireCodes(racine, "", codes);

    ofstream sortie(FICHIER_HUFFMAN.c_str(), ios::binary);

    if (!sortie)
    {
        libererArbre(racine);
        cerr << "Erreur : impossible de creer mots.huff" << endl;
        return false;
    }

    // En-tete : signature puis frequences des 256 octets.
    sortie.write("HUF1", 4);

    for (int i = 0; i < 256; i++)
    {
        sortie.write(
            reinterpret_cast<const char*>(&frequences[i]),
            sizeof(frequences[i])
        );
    }

    cout << "  Encodage des donnees..." << endl;

    entree.open(FICHIER_SOURCE.c_str(), ios::binary);

    if (!entree)
    {
        sortie.close();
        libererArbre(racine);
        return false;
    }

    EcrivainBits bits(sortie);
    uint64_t caracteresTraites = 0;

    while (entree.get(c))
    {
        unsigned char caractere = static_cast<unsigned char>(c);
        bits.ecrireCode(codes[caractere]);
        caracteresTraites++;

        if (caracteresTraites % 10000000 == 0)
        {
            cout << "  " << caracteresTraites
                 << " caracteres compresses." << endl;
        }
    }

    bits.terminer();

    entree.close();
    sortie.close();

    ifstream fichierMesure(FICHIER_HUFFMAN.c_str(),
                           ios::binary | ios::ate);

    if (!fichierMesure)
    {
        libererArbre(racine);
        return false;
    }

    tailleCompressee = static_cast<uint64_t>(fichierMesure.tellg());
    fichierMesure.close();

    auto fin = steady_clock::now();
    tempsCompression = duration<double>(fin - debut).count();

    libererArbre(racine);

    cout << "Compression terminee." << endl;

    return true;
}

// =====================================================
// 11. TAILLE D'UN FICHIER
// =====================================================

uint64_t tailleFichier(const string& nom)
{
    ifstream fichier(nom.c_str(), ios::binary | ios::ate);

    if (!fichier)
        return 0;

    return static_cast<uint64_t>(fichier.tellg());
}

// =====================================================
// 12. PROGRAMME PRINCIPAL
// =====================================================

int main()
{
    cout << "========================================" << endl;
    cout << " RECHERCHE ET COMPRESSION HUFFMAN" << endl;
    cout << "========================================" << endl;

    uint64_t nombreMots = NB_MOTS_TEST;

    cout << "\nNombre de mots a generer : " << nombreMots << endl;

    string sousChaine;
    string remplacement;

    cout << "Sous-chaine a rechercher (exemple : com) : ";
    cin >> sousChaine;

    cout << "Texte de remplacement (exemple : XYZ) : ";
    cin >> remplacement;

    if (sousChaine.empty())
    {
        cout << "La sous-chaine ne doit pas etre vide." << endl;
        return 1;
    }

    auto debutProgramme = steady_clock::now();

    genererFichier(nombreMots);

    if (tailleFichier(FICHIER_SOURCE) == 0)
    {
        cerr << "Erreur : mots.txt est vide ou inaccessible." << endl;
        return 1;
    }

    uint64_t trouvesFind = 0;
    uint64_t trouvesManuel = 0;

    double tempsFind = 0;
    double tempsManuel = 0;

    if (!traiterFichier(sousChaine, remplacement, true,
                        trouvesFind, tempsFind))
    {
        return 1;
    }

    if (!traiterFichier(sousChaine, remplacement, false,
                        trouvesManuel, tempsManuel))
    {
        return 1;
    }

    if (trouvesFind != trouvesManuel)
    {
        cout << "\nATTENTION : les nombres de mots contenant la sous-chaine "
             << "sont differents !" << endl;
    }
    else
    {
        cout << "\nVerification : les deux methodes trouvent le meme "
             << "nombre de mots contenant la sous-chaine." << endl;
    }

    uint64_t tailleOriginale = 0;
    uint64_t tailleCompressee = 0;
    double tempsCompression = 0;

    if (!compresserHuffman(tailleOriginale,
                          tailleCompressee,
                          tempsCompression))
    {
        return 1;
    }

    auto finProgramme = steady_clock::now();

    double tempsTotalProgramme =
        duration<double>(finProgramme - debutProgramme).count();

    double tauxCompression = 0;

    if (tailleOriginale > 0)
    {
        tauxCompression =
            100.0 * (1.0 -
            static_cast<double>(tailleCompressee) / tailleOriginale);
    }

    cout << "\n[5/5] RESULTATS FINAUX" << endl;
    cout << "----------------------------------------" << endl;

    cout << "Mots generes                 : " << nombreMots << endl;
    cout << "Mots contenant la sous-chaine (find) : "
         << trouvesFind << endl;
    cout << "Mots contenant la sous-chaine (manuel) : "
         << trouvesManuel << endl;

    cout << fixed << setprecision(3);
    cout << "Temps find + remplacement    : "
         << tempsFind << " s" << endl;
    cout << "Temps manuel + remplacement : "
         << tempsManuel << " s" << endl;
    cout << "Temps compression Huffman   : "
         << tempsCompression << " s" << endl;

    cout << "Taille originale             : "
         << tailleOriginale << " octets" << endl;
    cout << "Taille compressee            : "
         << tailleCompressee << " octets" << endl;
    cout << "Reduction de taille          : "
         << tauxCompression << " %" << endl;
    cout << "Temps total du programme     : "
         << tempsTotalProgramme << " s" << endl;

    cout << "\nFichiers crees :" << endl;
    cout << "- mots.txt" << endl;
    cout << "- resultats_find.txt" << endl;
    cout << "- resultats_manuel.txt" << endl;
    cout << "- mots.huff" << endl;

    cout << "\nProgramme termine avec succes !" << endl;

    return 0;
}
