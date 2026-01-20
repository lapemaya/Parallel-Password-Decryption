#include <chrono>
#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <random>

    //////////////////////
    /// MAIN FUNCTION ///
    /// /////////////////
int main() {

    //// HASH VARIABLES ////
    const std::string salt = "AB";
    const char* salt_cstr = salt.c_str();
    std::string found;
    bool found_flag = false;
    std::random_device rd;
    std::mt19937 gen(rd());

    //// TIMER START ////
    const auto start = std::chrono::high_resolution_clock::now();

    //// MAIN LOOP VARIABLES ////
    constexpr int NUM_ITER = 500;
    constexpr long long PASSWORDS_PER_ITER = 32LL * 13 * 2026;
    long long total_passwords_tested = 0;
    int correct_matches = 0;
    int incorrect_matches = 0;

    //// MAIN LOOP ////
    for (int i = 0; i < NUM_ITER; i++) {
        found = "";
        found_flag = false;

        //// GENERATE RANDOM PASSWORD TARGET ////
        std::uniform_int_distribution<int> giorno(1, 31);
        std::uniform_int_distribution<int> mese(1, 12);
        std::uniform_int_distribution<int> anno(0, 2025);

        const int g = giorno(gen);
        const int m = mese(gen);
        const int y = anno(gen);

        char target_password[9];
        target_password[0] = '0' + (g / 10);
        target_password[1] = '0' + (g % 10);
        target_password[2] = '0' + (m / 10);
        target_password[3] = '0' + (m % 10);
        target_password[4] = '0' + ((y / 1000) % 10);
        target_password[5] = '0' + ((y / 100) % 10);
        target_password[6] = '0' + ((y / 10) % 10);
        target_password[7] = '0' + (y % 10);
        target_password[8] = '\0';

        //// CRYPT TARGET PASSWORD ////
        const char* target = crypt(target_password, salt_cstr);
        const std::string target_string = target;

        //// BRUTE FORCE SEARCH ////
        for (int a = 0; a <= 31 && !found_flag; ++a) {
            for (int b = 0; b <= 12 && !found_flag; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    if (found_flag) break;

                    char date[9];
                    date[0] = '0' + (a / 10);
                    date[1] = '0' + (a % 10);
                    date[2] = '0' + (b / 10);
                    date[3] = '0' + (b % 10);
                    date[4] = '0' + ((c / 1000) % 10);
                    date[5] = '0' + ((c / 100) % 10);
                    date[6] = '0' + ((c / 10) % 10);
                    date[7] = '0' + (c % 10);
                    date[8] = '\0';

                    //// CRYPT GENERATED DATE ////
                    const char* h = crypt(date, salt_cstr);

                    //// CHECK IF HASH MATCHES TARGET ////
                    if (h[2] == target[2] && h[3] == target[3]) {
                        if (strcmp(h, target_string.c_str()) == 0) {
                            found = date;
                            found_flag = true;
                            break;
                        }
                    }
                }
            }
        }

        //// UPDATE STATS ////
        total_passwords_tested += PASSWORDS_PER_ITER;
        if (!found.empty()) {
            if (strcmp(found.c_str(), target_password) == 0) {
                correct_matches++;
            } else {
                incorrect_matches++;
            }
        }
    }
    //// TIMER END ////
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    double passwords_per_second = total_passwords_tested / elapsed.count();


    //// PRINT RESULTS ////
    std::cout << "========================================\n";
    std::cout << "Elaborazione completata!\n";
    std::cout << "========================================\n";
    std::cout << "Tempo totale impiegato: " << std::fixed << std::setprecision(2) << elapsed.count() << " secondi\n";
    std::cout << "Tempo medio per iterazione: " << std::fixed << std::setprecision(3) << (elapsed.count()/NUM_ITER) << " secondi\n";
    std::cout << "Iterazioni totali: " << NUM_ITER << "\n";
    std::cout << "Password testate totali: " << total_passwords_tested << "\n";
    std::cout << "Password testate/secondo: " << std::fixed << std::setprecision(0) << passwords_per_second << "\n";
    std::cout << "Thread utilizzati: 1 (sequenziale)\n";
    std::cout << "========================================\n";
    std::cout << "VERIFICA CORRETTEZZA:\n";
    std::cout << "Password corrette trovate: " << correct_matches << "/" << NUM_ITER << "\n";
    std::cout << "Password errate trovate: " << incorrect_matches << "/" << NUM_ITER << "\n";


    //// FINAL CHECK CORRECTNESS ////
    if (correct_matches == NUM_ITER) {
        std::cout << "✓ SUCCESSO: Tutte le password sono state trovate correttamente!\n";
    } else {
        std::cout << "✗ ERRORE: Alcune password non sono state trovate o sono errate!\n";
    }
    if (!found.empty()) {
        printf("✓ Ultima password trovata: %s\n", found.c_str());
    } else {
        printf("✗ Ultima password non trovata\n");
    }
    return 0;
}