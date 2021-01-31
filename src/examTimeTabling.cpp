// ανάγνωση δεδομένων από ένα αρχείο προβλήματος που περιέχει
// πληροφορίες για τις εξετάσεις μαθημάτων στις οποίες είναι εγγεγραμμένος
// ο κάθε σπουδαστής

#include <map>
#include <fstream>
#include <iostream> 
#include <sstream>
#include <vector>
#include <set> 
#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>
using namespace std;

// Δήλωση των vectors που θα χρησιμοποιηθούν
vector<int> exam_common_matrix_shuffled;
vector<int> assigned_colors;
vector<bool> available_colors;
vector<int> temp_shuffle;
vector<int> exam_common_matrix;
vector<int> exam_common_matrix_sorted;

// Kαταμέτρηση των κοινών στοιχείων
int count_common_elements(set<int> s1, set<int> s2)
{
    int c = 0;
    for (int x : s1) // range based for
    {
        for (int y : s2)
        {
            if (x == y)
            {
                c++;
                break;
            }
        }
    }
    return c;
}

void read_data(string fn, int students, int exams, int periods)
{

    // clear() -   "καθαρίζει" τα vectors για κάθε νέο πρόβλημα - έχουν δηλωθεί ως global  
    exam_common_matrix_shuffled.clear();
    assigned_colors.clear();
    available_colors.clear();
    temp_shuffle.clear();
    exam_common_matrix.clear();
    exam_common_matrix_sorted.clear();

    // ανάγνωση δεδομένων
    vector<set<int>> exam_students(exams + 1); // +1 επειδή ξεκινάει από το 0
    fstream fs(fn); //άνοιγμα του αρχείου
    if (!fs.is_open())
    {
        cerr << "Could not open file " << fn << std::endl;
        exit(-1);
    }
    int student_id = 0;
    string line;
    while (getline(fs, line))
    {
        if (line.empty())
            continue;
        student_id++;
        istringstream iss(line);
        int exam_id;
        while (iss >> exam_id)
        {
            exam_students[exam_id].insert(student_id);
        }
    }
    fs.close();


    // Δημιουργία πίνακα γειτνίασης
    map<pair<int, int>, int> adj_map;
    for (int i = 0; i < exams; i++)
    {
        for (int j = 0; j < exams; j++)
        {
            if (i == j) {
                adj_map[make_pair(i, j)] = 0;
                continue;
            }
            int c = count_common_elements(exam_students[i + 1], exam_students[j + 1]);
            adj_map[make_pair(i, j)] =  c;
        }
    }

    // Υπολογισμός συντελεστή πυκνότητας
    int c = 0;
    for (int i = 0; i < exams; i++)
    {
        for (int j = 0; j < exams; j++)
        {
            if (adj_map[make_pair(i, j)] > 0)
            {
                c++;
            }
        }
    }

    double conf_den= double(c) / double(exams * exams);
    
    
    // Η ΔΙΑΔΙΚΑΣΙΑ ΧΡΩΜΑΤΙΣΜΟΥ ΜΕΡΟΣ 1  
    // Χρήση αλγορίθμου Largest First 
    
    // Δημιουργία  διανύσματος (exam_common_matrix) για τον έλεγχο και τοποθέτηση 
    // των εξετάσεων που που έχουν κοινούς σπουδαστές 
    exam_common_matrix.clear();
    // Εκκίνηση με 0 παντού
    for (int i = 0; i < exams; i++) {
        exam_common_matrix.push_back(0);
    }
    // Έλεγχος όλων των συνδιασμοών μαθημάτων ώστε να βρεθεί 
    // ποιές εξετάσεις έχουν τις περισσότερες συγκρούσεις με άλλες εξετάσεις
    for (int i = 0; i < exams; i++)
    {
        for (int j = i+1; j < exams; j++)
        {
            if (adj_map[make_pair(i, j)] > 0)
            {
                exam_common_matrix[i] = exam_common_matrix[i] + 1; 
                exam_common_matrix[j] = exam_common_matrix[j] + 1; 
            }
        }
    }


    int max_conflict = -1;
    // Εύρεση εξέτασης με τις περισσότερες συγκρούσεις
    for (int i = 0; i < exams; i++) {
        if (max_conflict < exam_common_matrix[i])
        {
            max_conflict = exam_common_matrix[i];
        }
    }
    // Δημιουρία νέου διανύσματος για να τοποθετηθούν σε σειρά οι εξετάσεις 
    // από αυτές με τις περισσότερες συγκρούσεις ως τις λιγότερες
    exam_common_matrix_sorted.clear();
    // Προσθέτω στον πίνακα τις εξετάσεις με την σειρά προτεραιότητας
    for (int conflicts = max_conflict; conflicts > -1; conflicts--){
        for (int i = 0; i < exams; i++) {
            if (conflicts == exam_common_matrix[i]){
                exam_common_matrix_sorted.push_back(i);
            }
        }
    }

    
    // ΔΙΑΔΙΚΑΣΙΑ ΧΡΩΜΑΤΙΣΜΟΥ ΜΕΡΟΣ 2
    // Μέσω της while υποχρεώνεται να προσπαθεί να χρωματίσει με τον αριθμό τον περιόδων που διαθέτουμε
    bool success = false;

    while (success == false) {
        exam_common_matrix_shuffled.clear();
    // Διαδικασία ανακατέματος (shuffle)
        int z = 0;
    // Ανακάτεμα ανά 5%
        int percent = (exams * 5) / 100;    //Εύρεση του 5% των εξετάσεων
        temp_shuffle.clear();
        for (int i = 0; i < exams; i++) {
            temp_shuffle.push_back(exam_common_matrix_sorted[i]); //τοποθέτηση εξέτασης στο διάνυσμα
            z = z + 1;
            
        // Εαν έχουν μαζευτεί percent (5%) ή δεν υπάρχουν άλλες εξετάσεις
            if (z == percent || i == exams - 1) {
                // Random Generator
                unsigned num = chrono::system_clock::now().time_since_epoch().count();
                // Ανακάτεμα (shuffle)
                shuffle(temp_shuffle.begin(), temp_shuffle.end(), default_random_engine(num));

                for (int j = 0; j < z; j++) {
                    exam_common_matrix_shuffled.push_back(temp_shuffle[j]);
                }
                z = 0;
                temp_shuffle.clear();
            }
        }

    // Χρωματισμός
    // Εκκίνηση ενός vector με τα χρώματα να είναι όλα -1 δηλ. δεν εχουν ανατεθεί ακόμη
       assigned_colors.clear();
        for (int i = 0; i < exams; i++) {
            assigned_colors.push_back(-1);
        }

        for (int i = 0; i < exams; i++) {
            // Δημιουργία νέου vector όπου αρχικά όλα τα χρώματα είναι διαθέσιμα
            available_colors.clear();
            for (int z = 0; z < periods; z++) {
                available_colors.push_back(true);
            }
            // Έλεγχος για το  αν κάποια απο τις ήδη ανατεθείσες εξετάσεις συγκρούεται με αυτήν που εξετάζεται 
            for (int j = 0; j < i; j++) {
                // Εάν υπάρχουν κοινοί εξεταζόμενοι και αν έχει ανατεθεί χρώμα διαθεσση χρώματος  εκτός
                if (adj_map[make_pair(exam_common_matrix_shuffled[i], exam_common_matrix_shuffled[j])] > 0 && assigned_colors[j]!=-1)
                {
                    available_colors[assigned_colors[j]] = false;
                }
            }
            // Ανάθεση του πρώτου διαθέσιμου χρώματος
            for (int z = 0; z < periods; z++) {
                if (available_colors[z] == true){
                    assigned_colors[i] = z; 
                    break; // σταματάει όταν το βρίσκει
                }
            }
        }

        // Αφού έχουν διαπεραστεί όλες οι εξετάσεις ελέγχει αν πήραν όλες χρώμα 
        // και θέτει το success true ωστε τερματίσει η while
        bool end_while = true;
        for (int i = 0; i < exams; i++) {
            if (assigned_colors[i] == -1)
            {
                end_while = false;
                break;
            }
        }
        if (end_while == true) {
            success = true;

        }

    }

   
    // Βαθμολόγηση
    int score = 0;
    for (int i = 0; i < exams; i++) {
        for (int j = i + 1; j < exams; j++) {
        // Εάν η (απόλυτη) απόσταση των περιόδων δύο εξετάσεων ειναι μικρότερη ίση του 5
            int absolute_distance = abs(assigned_colors[i] - assigned_colors[j]);
            if (absolute_distance <= 5) {
                // Εάν έχουν κοινούς μαθητές
                int common = adj_map[make_pair(exam_common_matrix_shuffled[i] ,exam_common_matrix_shuffled[j])];
                if (common > 0)
                {
                    // Προσθήκη ποινής
                    if (absolute_distance == 1) {
                        score = score + common * 16;
                    } else if(absolute_distance == 2) {
                        score = score + common * 8;
                    }
                    else if (absolute_distance == 3) {
                        score = score + common * 4;
                    }
                    else if (absolute_distance == 4) {
                        score = score + common * 2;
                    }
                    else if (absolute_distance == 5) {
                        score = score + common * 1;
                    }
                }
            }
        }
    }

    // Διαίρεση του σκορ με τους σπουδαστές 
    double score_divided = double(score) / double(students);

     // Πλήθος περιόδοων που χρησιμοποιήθηκαν
    int periods_used = -1;
    for (int i=0; i<exams; i++){
        if(assigned_colors[i]>= periods_used){
            periods_used = assigned_colors[i];
        }
    }
    periods_used++;

    // Αποθήκευση αρχείου λύσης
    ofstream myfile;
    string savefile = fn.substr(0, fn.size() - 4);//Αφαίρεση κατάληξης .stu
    savefile.append("(" + to_string(score_divided) + ")");
    savefile.append(".sol"); // Προσθήκη κατάληξης .sol

    myfile.open(savefile);

    for (int i = 0; i < exams; i++) {
        myfile << exam_common_matrix_shuffled[i] + 1 << "\t" << assigned_colors[i] << endl;
    }
    myfile.close();

    // Εκτύπωση αποτελέσματος
    cout << "\n" << endl;
    cout <<"------------------------------------------------------" << endl;
    cout <<"Apothikeytike : "<<savefile <<endl;
    cout << "Conflict Density: " << conf_den << endl;
    cout <<"Periodoi : "<<periods_used <<endl;
    cout <<"------------------------------------------------------" << endl;
    cout << "\n" << endl; 
}


int main()
{
int choice;
do{
    cout <<"-----Problems----- \n";
    cout << "\n" << endl;  
    cout <<"1. car-f-92.stu \n";
    cout <<"2. car-s-91.stu \n";
    cout <<"3. ear-f-83.stu \n";
    cout <<"4. hec-s-92.stu \n";
    cout <<"5. kfu-s-93.stu \n";
    cout <<"6. lse-f-91.stu \n";
    cout <<"7. pur-s-93.stu \n";
    cout <<"8. rye-s-93.stu \n";
    cout <<"9. sta-f-83.stu \n";
    cout <<"10. tre-s-92.stu \n";
    cout <<"11. uta-s-92.stu \n";
    cout <<"12. ute-s-92.stu \n";
    cout <<"13. Myor-f-83.stu \n";
    cout <<"14. Maziki epilysi provlimatwn \n";
    cout <<"0. EXIT\n";
    cout <<"\n Please enter a choice: ";
    
    cin >> choice;
    switch(choice) {
    case 1:
        read_data("../datasets/car-f-92.stu", 18419, 543, 32);
    break;
    case 2:
        read_data("../datasets/car-s-91.stu", 16925, 682, 35);
    break;
    case 3:
        read_data("../datasets/ear-f-83.stu", 1125, 190, 24);
    break;
    case 4:
        read_data("../datasets/hec-s-92.stu", 2823, 81, 18);
    break;
    case 5:
        read_data("../datasets/kfu-s-93.stu", 5349, 461, 20);
    break;
    case 6:
        read_data("../datasets/lse-f-91.stu", 2726, 381, 18);
    break;
    case 7:
        read_data("../datasets/pur-s-93.stu", 30029, 2419, 42);
    break;
    case 8:
        read_data("../datasets/rye-s-93.stu", 11483, 486, 23);
    break;
    case 9:
    read_data("../datasets/sta-f-83.stu", 611, 139, 13);
    break;
    case 10:
        read_data("../datasets/tre-s-92.stu", 4360, 261, 23);
    break;
    case 11:
        read_data("../datasets/uta-s-92.stu", 21266, 622, 35);
    break;
    case 12:
        read_data("../datasets/ute-s-92.stu", 2749, 184, 10);
    break;
    case 13:
        read_data("../datasets/yor-f-83.stu", 941, 181, 21);
    break;
    case 14:
        read_data("../datasets/car-f-92.stu", 18419, 543, 32);
        read_data("../datasets/car-s-91.stu", 16925, 682, 35);
        read_data("../datasets/ear-f-83.stu", 1125, 190, 24);
        read_data("../datasets/hec-s-92.stu", 2823, 81, 18);
        read_data("../datasets/kfu-s-93.stu", 5349, 461, 20);
        read_data("../datasets/lse-f-91.stu", 2726, 381, 18);
        read_data("../datasets/pur-s-93.stu", 30029, 2419, 42);
        read_data("../datasets/rye-s-93.stu", 11483, 486, 23);
        read_data("../datasets/sta-f-83.stu", 611, 139, 13);
        read_data("../datasets/tre-s-92.stu", 4360, 261, 23);
        read_data("../datasets/uta-s-92.stu", 21266, 622, 35);
        read_data("../datasets/ute-s-92.stu", 2749, 184, 10);
        read_data("../datasets/yor-f-83.stu", 941, 181, 21);  
    case 0:
        cout <<" " <<endl;
    break;
    default:
        cout << "Invalid selection! Try again" <<endl; 
    }
} while(choice!=0);

}



