#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <sstream>

// Les différents type de valeurs possibles
enum BencodeType
{
    Integer,
    String,
    List,
    Dictionary
};

// Structure pour représenter une valeur Bencode
struct bencodedVal
{
    BencodeType type;
    std::variant<long long, std::string, std::vector<bencodedVal>, std::map<std::string, bencodedVal>> value;
};

// Fonction pour décoder une chaîne Bencode
std::string decodeString(std::istream& input);

// Fonction pour décoder une liste Bencode
std::vector<bencodedVal> decodeList(std::istream& input);

// Fonction pour décoder un dictionnaire Bencode
std::map<std::string, bencodedVal> decodeDictionary(std::istream& input);

// Fonction pour décoder une valeur Bencode
bencodedVal decodeBencode(std::istream& input);

// Fonction pour décoder les données Bencode et retourner la valeur décodée
bencodedVal decryptBencodedData(const std::string& bencodedData);

// Fonction pour afficher les informations d'un fichier torrent 
void displayTorrentInfo(const bencodedVal& torrentData);

// Fonction pour décoder une chaîne Bencode
std::string decodeString(std::istream& input)
{
    std::string stringLength;
    char someRandomAssVar;

    // lis les char jusqu'a ce que e soit rencontré
    while (input.get(someRandomAssVar) && someRandomAssVar != ':')
    {
        stringLength += someRandomAssVar;
    }

    // Cast la longueur de la chaîne en entier
    int length = std::stoi(stringLength);

    // Crée une string qui de la taille requise 
    std::string str(length, ' ');
    input.read(&str[0], length);

    return str;
}

// Fonction pour décoder une liste Bencode
std::vector<bencodedVal> decodeList(std::istream& input)
{
    std::vector<bencodedVal> list;
    char c;

    // Lire les éléments jusqu'à ce que e soit spotted
    while (input.peek() != 'e')
    {
        bencodedVal element = decodeBencode(input);
        list.push_back(element);
    }

    // vire le e
    input.get(c);

    return list;
}

// Fonction pour décoder un dictionnaire Bencode
std::map<std::string, bencodedVal> decodeDictionary(std::istream& input)
{
    std::map<std::string, bencodedVal> dictionary;
    char c;

    // Lire les paires clé-valeur jusqu'à ce que e soit rencontré
    while (input.peek() != 'e')
    {
        std::string key = decodeString(input);
        bencodedVal value = decodeBencode(input);
        dictionary[key] = value;
    }

    // On vire le e vu qu'il sert juste d'indicateur de type
    input.get(c);

    return dictionary;
}

// Fonction pour décoder une valeur Bencode
bencodedVal decodeBencode(std::istream& input)
{
    char c;

    //plus de caractères, retourner une valeur entière de 0
    if (!input.get(c))
    {
        return bencodedVal{ BencodeType::Integer, 0 };
    }

    // Vérifier le type de valeur Bencode en fonction du premier caractère, voir le lien de la doc en haut du code.
    switch (c)
    {
    case 'i': // Int
    {
        std::string integerStr;
        while (input.get(c) && c != 'e')
        {
            integerStr += c;
        }
        return bencodedVal{ BencodeType::Integer, std::stoll(integerStr) };
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': // string
    {
        input.unget();
        return bencodedVal{ BencodeType::String, decodeString(input) };
    }
    case 'l': // Liste
    {
        return bencodedVal{ BencodeType::List, decodeList(input) };
    }
    case 'd': // Dictionnaire
    {
        return bencodedVal{ BencodeType::Dictionary, decodeDictionary(input) };
    }
    default: // Type invalide, retourner une valeur entiere de 0
    {
        return bencodedVal{ BencodeType::Integer, 0 };
    }
    }
}

// Fonction pour décoder les données Bencode et retourner la valeur décodée
bencodedVal decryptBencodedData(const std::string& bencodedData)
{
    std::istringstream input(bencodedData);
    return decodeBencode(input);
}

// Fonction pour afficher les informations d'un fichier torrent Bencode
void displayTorrentInfo(const bencodedVal& torrentData)
{
    if (torrentData.type == BencodeType::Dictionary)
    {
        const std::map<std::string, bencodedVal>& dictionary = std::get<std::map<std::string, bencodedVal>>(torrentData.value);
        for (const auto& entry : dictionary)
        {
            std::cout << entry.first << ": ";
            switch (entry.second.type)
            {
            case BencodeType::Integer:
                std::cout << std::get<long long>(entry.second.value);
                break;
            case BencodeType::String:
                std::cout << std::get<std::string>(entry.second.value);
                break;
            case BencodeType::List:
                std::cout << "[Liste]";
                break;
            case BencodeType::Dictionary:
                std::cout << "[Dictionnaire]";
                break;
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Fichier torrent incorrect !" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << ">> Glissez et déposez le fichier torrent Bencode sur l'exécutable pour afficher ses informations." << std::endl;
        return 0;
    }

    // Drag and drop sur l'exe en utilisant l'index de argv
    std::string filePath = argv[1];

    // Lis le contenu du fichier torrent
    std::ifstream file(filePath, std::ios::binary);

    if (!file)
    {
        std::cout << "Erreur lors de la lecture du fichier torrent Bencode." << std::endl;
        return 0;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string bencodedData = oss.str();

    // Décoder les données Bencode
    bencodedVal torrentData = decryptBencodedData(bencodedData);

    // Afficher les informations du fichier torrent Bencode
    displayTorrentInfo(torrentData);
    std::cin.ignore();


    return 0;
}