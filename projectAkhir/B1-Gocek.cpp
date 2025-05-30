#include <iostream> 
#include <vector> //array dinamis
#include <map>
#include <regex> //error handling
#include <algorithm>
#include <ctime> //interval
#include <limits>
#include <iomanip> //fixed, precision
#include "tabulate/table.hpp"

using namespace std;
using namespace tabulate;

enum Role {PEMBELI, KASIR, ADMIN};

struct User {
    string username, password;
    Role role;
    bool blocked = false;
};

struct Product {
    int id;
    string nama, jenis;
    int stok, pembelian;
    double harga;
};

struct keranjangBelanja {
    int product_id;
    int qty;
};

struct Invoice {
    int id;
    string username;
    vector<keranjangBelanja> items;
    double total;
    string status; //pending/berhasil
    time_t waktu;
};

string getString(const string& prompt) {  //io bareng
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

vector<User> users;
vector<Product> products;
vector<Invoice> invoices;
map<string, vector<keranjangBelanja>> keranjang; // username -> keranjang

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


void printTableProduk(const vector<Product>& prods) {
    Table t;
    t.add_row({"ID", "Nama", "Jenis", "Harga (Rp)", "Stok", "Pembelian"});
    for (const auto& p : prods) {
        stringstream ssHarga;
        ssHarga << fixed << setprecision(2) << p.harga;
        t.add_row(
            {to_string(p.id),
            p.nama, p.jenis,
            ssHarga.str(), 
            to_string(p.stok), 
            to_string(p.pembelian)});
    }
    cout << t << endl;
}

void printTableKeranjang(const vector<keranjangBelanja>& cart) {
    Table t;
    t.add_row({"ID", "Nama", "Qty"});
    for (const auto& ci : cart) {
        auto it = find_if(products.begin(), products.end(),
                        [&](const Product& p) { return p.id == ci.product_id; });
        if (it != products.end())
            t.add_row({to_string(it->id), it->nama, to_string(ci.qty)});
    }
    cout << t << endl;
}

void printTableInvoice(const Invoice& inv) {
    Table t;
    t.add_row({"ID", "Nama Produk", "Qty", "Harga"});
    for (const auto& ci : inv.items) {
        auto it = find_if(products.begin(), products.end(),
                        [&](const Product& p) { return p.id == ci.product_id; });
        if (it != products.end())
            t.add_row({to_string(it->id), it->nama, to_string(ci.qty), to_string((int)it->harga)});
    }
    cout << t << endl;
    cout << "Total: " << inv.total << endl;
    cout << "Status: " << inv.status << endl;
    cout << "No Rekening: 1234567890 (BCA) a.n. GOCEK\n";
}

//validasi
bool validasiUname(const string& uname) {
    regex pattern("^[a-zA-Z0-9!@#$%^&*()_+\\-=\\[\\]{};:'\",.<>/?]+$");
    return regex_match(uname, pattern);
}

bool validasiPass(const string& pass) {
    regex pattern("^[a-zA-Z0-9!@#$%^&*()_+\\-=\\[\\]{};:'\",.<>/?]+$");
    return regex_match(pass, pattern);
}

bool validasiNamaProduk(const string& nama) {
    regex pattern("^[a-zA-Z0-9\\s]+$"); //huruf angka spaso
    return regex_match(nama, pattern) && !nama.empty();
}

bool validasiAngka(const string& input) {
    regex pattern("^[0-9]+$"); //angka >0
    return regex_match(input, pattern) && !input.empty();
}

bool validasiAngkaDecimal(const string& input) {
    regex pattern("^[0-9]+(\\.[0-9]+)?$"); //desimal
    return regex_match(input, pattern) && !input.empty();
}

bool isUsernameUnique(const string& uname) {
    for (const auto& u : users) {
        if (u.username == uname) return false;
    }
    return true;
}

User* getUser(const string& uname) {
    for (auto& u : users)
        if (u.username == uname)
            return &u;
    return nullptr;
}

Product* getProduct(int id) {
    for (auto& p : products)
        if (p.id == id)
            return &p;
    return nullptr;
}

void bubbleSort(vector<Product>& prods, int mode, bool asc) {
    for (size_t i = 0; i < prods.size(); ++i) {
        for (size_t j = 0; j < prods.size() - i - 1; ++j) {
            bool cond = false;
            switch (mode) {
                case 1:
                    cond = asc ? prods[j].harga > prods[j + 1].harga : prods[j].harga < prods[j + 1].harga;
                    break;
                case 2:
                    cond = asc ? prods[j].nama > prods[j + 1].nama : prods[j].nama < prods[j + 1].nama;
                    break;
                case 3:
                    cond = asc ? prods[j].stok > prods[j + 1].stok : prods[j].stok < prods[j + 1].stok;
                    break;
                case 4:
                    cond = asc ? prods[j].pembelian > prods[j + 1].pembelian : prods[j].pembelian < prods[j + 1].pembelian;
                    break;
            }
            if (cond) swap(prods[j], prods[j + 1]);
        }
    }
}

void tabelKasir (Table &menuTable) {
    menuTable.add_row({"Menu Kasir"});
    menuTable.add_row({"1. Lihat Produk"});
    menuTable.add_row({"2. Pencatatan Transaksi"});
    menuTable.add_row({"3. Riwayat Transaksi"});
    menuTable.add_row({"0. Keluar"});
}

void tabelAdmin (Table &menuTable) {
    menuTable.add_row({"Menu Admin"});
    menuTable.add_row({"1. Kelola Akun"});
    menuTable.add_row({"2. Konfirmasi Pembelian"});
    menuTable.add_row({"3. Lihat Laporan Bulanan"});
    menuTable.add_row({"0. Keluar"});
}

void tabelMainMenu (Table &menuTable) {
    menuTable.add_row({"Main Menu"});
    menuTable.add_row({"1. Login"});
    menuTable.add_row({"2. Register (member)"});
    menuTable.add_row({"0. Exit Program"});
}

void tabelPembeli (Table &menuTable) {
    menuTable.add_row({"Main Menu"});
    menuTable.add_row({"1. Beli Barang"});
    menuTable.add_row({"2. Histori Pembelian"});
    menuTable.add_row({"0. Keluar"});
}

int binarySearch(const vector<Product>& prods, string nama) {
    int l = 0, r = prods.size() - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        if (prods[m].nama == nama)
            return m;
        else if (prods[m].nama < nama)
            l = m + 1;
        else
            r = m - 1;
    }
    return -1;
}

//filter
vector<Product> filterByHarga(double min, double max) {
    vector<Product> res;
    for (const auto& p : products)
        if (p.harga >= min && p.harga <= max)
            res.push_back(p);
    return res;
}

vector<Product> filterByJenis(const string& jenis) {
    vector<Product> res;
    for (const auto& p : products)
        if (p.jenis == jenis)
            res.push_back(p);
    return res;
}

double getValidDouble(const string& prompt, double minVal = 0.01) {
    string input;
    double value;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (!validasiAngkaDecimal(input)) {
            cout << "Input tidak valid! Hanya boleh menggunakan angka dan titik desimal.\n";
            continue;
        }
        
        try {
            value = stod(input);
            if (value < minVal) {
                cout << "Input tidak valid! Nilai harus minimal " << minVal << ".\n";
                continue;
            }
            return value;
        } catch (const exception&) {
            cout << "Input tidak valid! Angka tidak dapat diproses.\n";
            continue;
        }
    }
}

int getValidInteger(const string& prompt, int minVal = 1) {
    string input;
    int value;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (!validasiAngka(input)) {
            cout << "Input tidak valid! Hanya boleh menggunakan angka (0-9).\n";
            continue;
        }
        
        try {
            value = stoi(input);
            if (value < minVal) {
                cout << "Input tidak valid! Nilai harus minimal " << minVal << ".\n";
                continue;
            }
            return value;
        } catch (const exception&) {
            cout << "Input tidak valid! Angka terlalu besar.\n";
            continue;
        }
    }
}

//rekursif
void tampilkanHistoryRec(const vector<Invoice>& invs, int idx, const string& uname) {
    if (idx >= invs.size()) return;
    if (invs[idx].username == uname) {
        cout << "Invoice ID: " << invs[idx].id << " | Total: " << invs[idx].total << " | Status: " << invs[idx].status << endl;
        cout << "Detail Produk:\n";
        printTableInvoice(invs[idx]); //detail produk yg dibeli
    }
    tampilkanHistoryRec(invs, idx + 1, uname);
}

int getInputMenu(int min, int max) {
    int pilihan;
    while (true) {
        cout << "Pilih: ";
        if (!(cin >> pilihan)) {
            cout << "Input tidak valid! Harap masukkan angka.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        if (pilihan < min || pilihan > max) {
            cout << "Pilihan harus antara " << min << " sampai " << max << ".\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return pilihan;
    }
}
//print status
void printTableUsers() {
    Table t;
    t.add_row({"Username", "Role", "Status"});
    for (const auto& u : users) {
        string roleStr = (u.role == ADMIN) ? "Admin" : (u.role == KASIR) ? "Kasir" : "Pembeli";
        string statusStr = u.blocked ? "Terblokir" : "Aktif";
        t.add_row({u.username, roleStr, statusStr});
    }
    cout << t << endl;
}

void printTablePendingInvoices() {
    Table t;
    t.add_row({"ID Invoice", "Username", "Total", "Status"});
    for (const auto& inv : invoices) {
        if (inv.status == "Pending") {
            t.add_row({to_string(inv.id), inv.username, to_string((int)inv.total), inv.status});
        }
    }
    cout << t << endl;
}


void registerMember() {
    clearScreen();
    cout << "Register Member ";
    string uname, pass;
    uname = getString("Username : ");
    if (!validasiUname(uname)) {
        cout << "Username tidak valid!\n";
        system("pause");
        return; 
    }
    if (!isUsernameUnique(uname)) {
        cout << "Username sudah ada!\n";
        getString("Tekan Enter Untuk Kembali");
        return;
    }
    pass = getString("Password : ");
    if (!validasiPass(pass)) {
        cout << "Password tidak valid!\n";
        system("pause");
        return;
    }
    users.push_back({uname, pass, PEMBELI, false});
    cout << "Register berhasil!\n";
    system("pause");
}

User* login() {
    clearScreen();
    cout << "Login ";
    string uname, pass;
    uname = getString("Username : ");
    User* u = getUser(uname);
    if (!u) {
        cout << "Username tidak ditemukan!\n";
        system("pause");
        return nullptr;
    }
    if (u->blocked) {
        cout << "Akun terblokir!\n";
        system("pause");
        return nullptr;
    }
    int tries = 3;
    while (tries--) {
        pass = getString("Password : ");
        if (u->password == pass)
            return u;
        cout << "Password salah! Sisa percobaan: " << tries << endl;
    }
    u->blocked = true;
    cout << "Akun terblokir!\n";
    system("pause");
    return nullptr;
}


void menuBeliBarang(User* user) {
    int sub;
    vector<Product> prods = products;
    vector<keranjangBelanja>& cart = keranjang[user->username];
    bool asc = true;
    while (true) {
        clearScreen();
        cout << "List Produk\n";
        printTableProduk(prods);
        cout << "Menu:\n1. Searching\n2. Sorting\n3. Filter\n4. Tambah Produk\n5. Lihat Keranjang\n6. Checkout\n0. Keluar\n";
        sub = getInputMenu(0, 6);
        if (sub == 0) break;
        if (sub == 1) {
            string nama;
            nama = getString("Nama Produk : ");
            bubbleSort(prods, 2, true);
            int idx = binarySearch(prods, nama);
            if (idx != -1) {
                cout << "Produk ditemukan:\n";
                printTableProduk({prods[idx]});
            } else
                cout << "Tidak ditemukan!\n";
            system("pause");
        } else if (sub == 2) {
            int m;
            cout << "Sorting berdasarkan:\n1. Harga\n2. Nama\n3. Stok\n4. Pembelian\n0. Keluar\nPilih: ";
            m = getInputMenu(0, 4);
            if (m == 0) continue;
            bubbleSort(prods, m, asc);
            printTableProduk(prods);
            cout << "Menu:\n1. Tukar Urutan Sorting\n0. Keluar\nPilih: ";
            int t = getInputMenu(0, 1);
            if (t == 1) {
                asc = !asc;
                bubbleSort(prods, m, asc);
            }
        } else if (sub == 3) {
            int f;
            cout << "Filter:\n1. Range Harga\n2. Jenis\n0. Kembali\nPilih: ";
            f = getInputMenu(0, 2);
            if (f == 0) continue;
            if (f == 1) {
                double min, max;
                cout << "Min: ";
                while (!(cin >> min)) {
                    cout << "Input tidak valid! Masukkan angka: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                cout << "Max: ";
                while (!(cin >> max)) {
                    cout << "Input tidak valid! Masukkan angka: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                vector<Product> res = filterByHarga(min, max);
                printTableProduk(res);
                system("pause");
            } else if (f == 2) {
                cout << "Jenis:\n1.Keyboard\n2.Mouse\n3.Mousepad\n4.Ram\n5.Fan\nPilih: ";
                int j = getInputMenu(1, 5);
                string jenis[] = {"Keyboard", "Mouse", "Mousepad", "Ram", "Fan"};
                vector<Product> res = filterByJenis(jenis[j - 1]);
                printTableProduk(res);
                system("pause");
            }
        } else if (sub == 4) {
            int id, qty;
            cout << "ID Produk: ";
            while (!(cin >> id)) {
                cout << "Input tidak valid! Masukkan angka: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Product* p = getProduct(id);
            if (!p) {
                cout << "Produk tidak ditemukan!\n";
                system("pause");
                continue;
            }
            cout << "Nama: " << p->nama << " | Stok: " << p->stok << " | Harga: " << p->harga << endl;
            cout << "Jumlah: ";
            while (!(cin >> qty)) {
                cout << "Input tidak valid! Masukkan angka: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (qty <= 0 || qty > p->stok) {
                cout << "Jumlah tidak valid atau melebihi stok!\n";
                system("pause");
                continue;
            }
            auto it = find_if(cart.begin(), cart.end(), [&](const keranjangBelanja& ci) { return ci.product_id == id; });
            if (it != cart.end()) {
                if (it->qty + qty > p->stok) {
                    cout << "Jumlah total di keranjang melebihi stok!\n";
                    system("pause");
                    continue;
                }
                it->qty += qty;
            } else {
                cart.push_back({id, qty});
            }
            cout << "Produk " << p->nama << " berhasil ditambahkan ke Keranjang!\n";
            system("pause");
        } else if (sub == 5) {
            clearScreen();
            cout << " Keranjang \n";
            printTableKeranjang(cart);
            cout << "Menu:\n1. Edit Produk\n0. Kembali\nPilih: ";
            int e = getInputMenu(0, 1);
            if (e == 1) {
                cout << "ID Produk: ";
                int id;
                while (!(cin >> id)) {
                    cout << "Input tidak valid! Masukkan angka: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                auto it = find_if(cart.begin(), cart.end(), [&](const keranjangBelanja& ci) { return ci.product_id == id; });
                if (it == cart.end()) {
                    cout << "Produk tidak ditemukan di keranjang!\n";
                    system("pause");
                    continue;
                }
                cout << "1. Tambah Qty\n2. Kurangi Qty\n3. Hapus Produk\nPilih: ";
                int op = getInputMenu(1, 3);
                if (op == 1) {
                    cout << "Tambah: ";
                    int x;
                    while (!(cin >> x)) {
                        cout << "Input tidak valid! Masukkan angka: ";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (x < 1) {
                        cout << "Kuantitas tambah harus minimal 1!\n";
                        system("pause");
                        continue; 
                    }
                    Product* p = getProduct(id);
                    if (p && it->qty + x > p->stok) {
                        cout << "Jumlah di keranjang melebihi stok!\n";
                        system("pause");
                        continue;
                    }
                    it->qty += x;
                } else if (op == 2) {
                    cout << "Kurangi: ";
                    int x;
                    while (!(cin >> x)) {
                        cout << "Input tidak valid! Masukkan angka: ";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (x > it->qty) {
                        cout << "Jumlah yang dikurangi melebihi jumlah di keranjang!\n";
                        system("pause");
                        continue;
                    }
                    if (x < 1) {
                        cout << "Kuantitas tambah harus minimal 1!\n";
                        system("pause");
                        continue; 
                    }
                    it->qty -= x;
                    if (it->qty <= 0) cart.erase(it);
                } else if (op == 3) {
                    cart.erase(it);
                }
            }
        } else if (sub == 6) {
            if (cart.empty()) {
                cout << "Keranjang kosong!\n";
                system("pause");
                continue;
            }
            string yn = getString("Apakah anda yakin ingin checkout? (Ya/Tidak): ");
            if (yn == "Ya" || yn == "ya" || yn == "YA") {
                double total = 0;
                bool stokCukup = true;
                for (auto& ci : cart) {
                    Product* p = getProduct(ci.product_id);
                    if (p) {
                        if (ci.qty > p->stok) {
                            cout << "Stok tidak cukup untuk produk " << p->nama << "!\n";
                            stokCukup = false;
                            break;
                        }
                        total += p->harga * ci.qty;
                    }
                }
                if (!stokCukup) {
                    system("pause");
                    continue;
                }
                int inv_id = invoices.size() + 1;
                invoices.push_back({inv_id, user->username, cart, total, "Pending", time(0)});
                cout << "INVOICE\n";
                printTableInvoice(invoices.back());
                cart.clear();
                system("pause");
            } else {
                cart.clear();
                cout << "Checkout dibatalkan.\n";
                system("pause");
            }
        }
    }
}

void menuPembeli(User* user) {
    while (true) {
        clearScreen();
        Table menuPb;
        tabelPembeli(menuPb);
        cout << menuPb << endl;
        int m = getInputMenu(0, 2);
        if (m == 0) break;
        if (m == 1)
            menuBeliBarang(user);
        else if (m == 2) {
            cout << "History Pembelian ";
            tampilkanHistoryRec(invoices, 0, user->username);
            system("pause");
        }
    }
}

void menuKasir(User* user) {
    while (true) {
        clearScreen();
        Table menuBener;
        tabelKasir(menuBener);
        cout << menuBener << endl;
        int m = getInputMenu(0, 3);
        if (m == 0) break;
        if (m == 1) {
            int sub;
            vector<Product> prods = products;
            bool asc = true;
            while (true) {
                clearScreen();
                cout << "List Produk \n";
                printTableProduk(prods);
                cout << "Menu:\n1. Searching\n2. Sorting\n3. Filter\n4. Kelola Produk\n0. Kembali\nPilih: ";
                sub = getInputMenu(0, 4);
                if (sub == 0) break;
                if (sub == 1) {
                    string nama;
                    nama = getString("Nama Produk : ");
                    bubbleSort(prods, 2, true);
                    int idx = binarySearch(prods, nama);
                    if (idx != -1)
                        printTableProduk({prods[idx]});
                    else
                        cout << "Tidak ditemukan!\n";
                    system("pause");
                } else if (sub == 2) {
                    int m2;
                    cout << "Sorting berdasarkan:\n1. Harga\n2. Nama\n3. Stok\n4. Pembelian\n0. Keluar\nPilih: ";
                    m2 = getInputMenu(0, 4);
                    if (m2 == 0) continue;
                    bubbleSort(prods, m2, asc);
                    printTableProduk(prods);
                    cout << "Menu:\n1. Tukar Urutan Sorting\n0. Keluar\nPilih: ";
                    int t = getInputMenu(0, 1);
                    if (t == 1) {
                        asc = !asc;
                        bubbleSort(prods, m2, asc);
                    }
                } else if (sub == 3) {
                    int f;
                    cout << "Filter:\n1. Range Harga\n2. Jenis\n0. Kembali\nPilih: ";
                    f = getInputMenu(0, 2);
                    if (f == 0) continue;
                    if (f == 1) {
                        double min_val, max_val; 
                        cout << "Min: ";
                        while (!(cin >> min_val) || min_val < 0) { 
                            cout << "Input tidak valid! Masukkan angka positif: ";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        cout << "Max: ";
                        while (!(cin >> max_val) || max_val < min_val) { 
                            cout << "Input tidak valid! Masukkan angka positif dan lebih besar atau sama dengan Min: ";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        vector<Product> res = filterByHarga(min_val, max_val);
                        printTableProduk(res);
                        system("pause");
                    } else if (f == 2) {
                        cout << "Jenis:\n1.Keyboard\n2.Mouse\n3.Mousepad\n4.Ram\n5.Fan\nPilih: ";
                        int j = getInputMenu(1, 5);
                        string jenis[] = {"Keyboard", "Mouse", "Mousepad", "Ram", "Fan"};
                        vector<Product> res = filterByJenis(jenis[j - 1]);
                        printTableProduk(res);
                        system("pause");
                    }
                } else if (sub == 4) {
                    int crud;
                    cout << "Menu:\n1. Tambah Produk\n2. Ubah Produk\n3. Hapus Produk\n0. Kembali\nPilih: ";
                    crud = getInputMenu(0, 3);
                    if (crud == 0) continue;

                    string jenisProduk[] = {"Keyboard", "Mouse", "Mousepad", "Ram", "Fan"}; 

                    if (crud == 1) { 
                        Product p;
                        p.id = products.empty() ? 1 : products.back().id + 1;
                        cout << "Nama: ";
                        string inputNama;
                        getline(cin, inputNama);

                        if (!validasiNamaProduk(inputNama)) {
                        cout << "Nama produk tidak valid! Hanya boleh menggunakan huruf, angka, dan spasi.\n";
                        system("pause");
                        continue;
                    }
                    p.nama = inputNama;

                        cout << "Pilih Jenis Produk:\n";
                        for (int i = 0; i < 5; ++i) {
                            cout << i + 1 << ". " << jenisProduk[i] << endl;
                        }
                        int jenisChoice = getInputMenu(1, 5);
                        p.jenis = jenisProduk[jenisChoice - 1];

                        p.harga = getValidDouble("Harga: ");

                        p.stok = getValidInteger("Stok: ");
                        
                        p.pembelian = 0;
                        products.push_back(p);
                        prods = products; 
                        cout << "Produk ditambah!\n";
                        system("pause");
                    } else if (crud == 2) { 
                        int id;
                        cout << "ID Produk yang akan diubah: ";
                        while (!(cin >> id)) {
                            cout << "Input tidak valid! Masukkan angka: ";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        Product* p_edit = getProduct(id); 
                        if (!p_edit) {
                            cout << "Produk tidak ditemukan!\n";
                            system("pause");
                            continue;
                        }
                        cout << "Data Produk Saat Ini:\n";
                        cout << "Nama: " << p_edit->nama << " | Jenis: " << p_edit->jenis << " | Harga: " << p_edit->harga << " | Stok: " << p_edit->stok << endl;

                        cout << "Nama baru (kosongkan jika tidak ingin mengubah): ";
                        string newNama;
                        getline(cin, newNama);
                        if (!newNama.empty()) {
                            p_edit->nama = newNama;
                        }
                        
                        cout << "Pilih Jenis Produk baru (0 jika tidak ingin mengubah):\n";
                        for (int i = 0; i < 5; ++i) {
                            cout << i + 1 << ". " << jenisProduk[i] << endl;
                        }
                        int jenisChoice = getInputMenu(0, 5);
                        if (jenisChoice != 0) {
                        p_edit->jenis = jenisProduk[jenisChoice - 1];
                        }

                        double newHarga = getValidDouble("Harga baru (masukkan 0 jika tidak ingin mengubah): ", 0);

                        if (newHarga > 0) {
                            p_edit->harga = newHarga;
                        }

                        int newStok = getValidInteger("Stok baru (masukkan 0 jika tidak ingin mengubah): ", 0); 
                        if (newStok > 0) { 
                            p_edit->stok = newStok;
                        } else if (newStok == 0 && p_edit->stok > 0) {
                            cout << "Stok tidak diubah (input 0). Stok saat ini: " << p_edit->stok << endl;
                        }


                        cout << "Produk diubah!\n";
                        prods = products; //copy
                        system("pause");
                    } else if (crud == 3) { 
                        int id;
                        cout << "ID Produk yang akan dihapus: ";
                        while (!(cin >> id)) {
                            cout << "Input tidak valid! Masukkan angka: ";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        auto it = remove_if(products.begin(), products.end(),
                                            [&](const Product& p_check) { return p_check.id == id; }); 
                        if (it != products.end()) {
                            products.erase(it, products.end());
                            prods = products; 
                            cout << "Produk dihapus!\n";
                        } else {
                            cout << "Produk dengan ID tersebut tidak ditemukan.\n";
                        }
                        system("pause");
                    }
                }
            }
        } else if (m == 2) { 
            clearScreen();
            cout << "List Produk\n";
            printTableProduk(products); 
            int id, qty;
            cout << "ID Produk: ";
            while (!(cin >> id)) {
                cout << "Input tidak valid! Masukkan angka: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            Product* p = getProduct(id);
            if (!p) {
                cout << "Produk tidak ditemukan!\n";
                system("pause");
                continue;
            }
            cout << "Nama: " << p->nama << " | Stok: " << p->stok << " | Harga: " << p->harga << endl;
            cout << "Jumlah: ";
            while (!(cin >> qty) || qty <= 0) { 
                cout << "Input tidak valid! Jumlah harus berupa angka lebih dari 0. Masukkan jumlah kembali: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

            if (qty > p->stok) {
                cout << "Jumlah melebihi stok yang tersedia (" << p->stok << ")!\n";
                system("pause");
                continue;
            }
            
            string yn = getString("Lanjutkan transaksi? (Ya/Tidak): "); 
            if (yn == "Ya" || yn == "ya" || yn == "YA") {
                p->stok -= qty;
                p->pembelian += qty;
                int inv_id = invoices.empty() ? 1 : invoices.back().id + 1; 
                invoices.push_back({inv_id, user->username, {{id, qty}}, p->harga * qty, "Berhasil", time(0)});
                cout << "Transaksi berhasil! Invoice:\n";
                printTableInvoice(invoices.back());
                system("pause");
            } else {
                cout << "Transaksi dibatalkan.\n";
                system("pause");
            }
        } else if (m == 3) { 
            clearScreen();
            cout << "Riwayat Transaksi untuk Kasir: " << user->username << endl;
            bool found = false;
            Table historyTable;
            historyTable.add_row({"ID Invoice", "Total", "Status", "Waktu", "Detail Produk"});

            for (const auto& inv : invoices) {
                if (inv.username == user->username) { 
                    found = true;
                    char bufferTanggal[80];
                    strftime(bufferTanggal, 80, "%Y-%m-%d %H:%M:%S", localtime(&inv.waktu));
                    
                    string detailProdukText;
                    for (const auto& ci : inv.items) {
                        Product* prod_info = getProduct(ci.product_id);
                        if (prod_info) {
                            detailProdukText += prod_info->nama + " (Qty: " + to_string(ci.qty) + ", Harga/item: " + to_string((int)prod_info->harga) + "); ";
                        } else {
                            detailProdukText += "ID Produk: " + to_string(ci.product_id) + " (Qty: " + to_string(ci.qty) + ") - Info produk tidak ditemukan; ";
                        }
                    }
                    if (!detailProdukText.empty()) { 
                        detailProdukText.pop_back();
                        detailProdukText.pop_back();
                    }
                    historyTable.add_row({to_string(inv.id), to_string((int)inv.total), inv.status, bufferTanggal, detailProdukText});
                }
            }

            if (!found) {
                cout << "Belum ada riwayat transaksi untuk kasir ini." << endl;
            } else {
                cout << historyTable << endl;
            }
            system("pause");
        }
    }
}

void hapusInvoiceKadaluarsa() {
    time_t waktuSekarang = time(0);
    const double BATAS_WAKTU_DETIK = 35; // 24 jam


    int jumlahSebelumnya = invoices.size();
    auto iteratorAkhirElemenValid = std::remove_if(invoices.begin(), invoices.end(),
        [&](const Invoice& inv) {
        if (inv.status == "Pending") {
            double selisihWaktu = difftime(waktuSekarang, inv.waktu);
            return selisihWaktu > BATAS_WAKTU_DETIK;
        }
        return false;
    });

    if (iteratorAkhirElemenValid != invoices.end()) {
        int jumlahDihapus = std::distance(iteratorAkhirElemenValid, invoices.end());
        invoices.erase(iteratorAkhirElemenValid, invoices.end());

        if (jumlahDihapus > 0) {
            cout << "\nINFO: " << jumlahDihapus << " invoice yang statusnya 'Pending' lebih dari 24 jam telah otomatis dihapus.\n";
            cout << "Tekan Enter untuk melanjutkan...";
            system("pause");
            
        }
    }
}


void menuAdmin(User* user) {
    while (true) {
        clearScreen(); 
        cout << "Menu Admin: " << user->username << endl;
        Table mnAdmin;
        tabelAdmin(mnAdmin); 
        cout << mnAdmin << endl; 
        int m = getInputMenu(0, 3); 

        if (m == 0) break; 

        if (m == 1) { 
            clearScreen(); 
            cout << "Kelola Akun Pengguna\n"; 
            printTableUsers(); 
            cout << "\nMenu Kelola Akun:\n1. Tambah Akun Kasir Baru\n2. Edit Akun (Username/Password/Blokir)\n3. Hapus Akun Pengguna\n0. Kembali\n"; 
            int sub = getInputMenu(0, 3); 

            if (sub == 0) continue; 

            if (sub == 1) { 
                User uNew;
                uNew.username = getString("Username untuk Kasir baru: "); 
                if (!validasiUname(uNew.username)) {
                    cout << "Username tidak valid.\n"; system("pause"); continue;
                }
                if (!isUsernameUnique(uNew.username)) { 
                    cout << "Username '" << uNew.username << "' sudah ada!\n"; 
                    system("pause"); 
                    continue; 
                }
                uNew.password = getString("Password untuk Kasir baru: "); 
                if (!validasiPass(uNew.password)) {
                    cout << "Password tidak valid.\n"; system("pause"); continue;
                }
                uNew.role = KASIR; 
                uNew.blocked = false; 
                users.push_back(uNew); 
                cout << "Akun kasir '" << uNew.username << "' berhasil ditambahkan!\n"; 
                system("pause"); 
                return;
            } else if (sub == 2) { //edit akun
                string unameEdit = getString("Masukkan Username akun yang akan diedit: "); 
                User* uEdit = getUser(unameEdit); 
                if (!uEdit) { 
                    cout << "Akun dengan username '" << unameEdit << "' tidak ditemukan!\n"; 
                    system("pause"); 
                    continue; 
                }
                cout << "Akun ditemukan: Username: " << uEdit->username << " | Role: " << (uEdit->role == ADMIN ? "Admin" : uEdit->role == KASIR ? "Kasir" : "Pembeli") << " | Status: " << (uEdit->blocked ? "Terblokir" : "Aktif") << endl;
                cout << "Pilih yang ingin diedit:\n1. Username Baru\n2. Password Baru\n3. Ubah Status Blokir (Buka/Blokir)\n0. Batal\n"; 
                int e = getInputMenu(0, 3); 
                if (e == 1) {
                    string newUname = getString("Masukkan Username baru: ");
                    if (!validasiUname(newUname)) { cout << "Username baru tidak valid.\n"; }
                    else if (!isUsernameUnique(newUname) && newUname != uEdit->username) { cout << "Username baru sudah ada.\n"; }
                    else { uEdit->username = newUname; cout << "Username berhasil diubah.\n"; } 
                } else if (e == 2) {
                    string newPass = getString("Masukkan Password baru: ");
                    if (!validasiPass(newPass)) { cout << "Password baru tidak valid.\n"; }
                    else { uEdit->password = newPass; cout << "Password berhasil diubah.\n"; } 
                } else if (e == 3) {
                    uEdit->blocked = !uEdit->blocked; //status blokir 
                    cout << "Status blokir akun '" << uEdit->username << "' telah diubah menjadi: " << (uEdit->blocked ? "Terblokir" : "Aktif") << ".\n"; 
                }
                system("pause"); 
            } else if (sub == 3) { 
                string unameHapus = getString("Masukkan Username akun yang akan dihapus: "); 
                if (unameHapus == user->username) { 
                    cout << "Anda tidak dapat menghapus akun Anda sendiri!\n";
                    system("pause");
                    continue;
                }
                auto it = remove_if(users.begin(), users.end(),
                                    [&](const User& u) { return u.username == unameHapus; }); 
                if (it != users.end()) { 
                    users.erase(it, users.end()); 
                    cout << "Akun '" << unameHapus << "' berhasil dihapus!\n"; 
                } else {
                    cout << "Akun dengan username '" << unameHapus << "' tidak ditemukan!\n";
                }
                system("pause"); 
            }
        } else if (m == 2) { //konfirmasi pembayaran invoice
            clearScreen(); 
            cout << "Konfirmasi Pembayaran Invoice\n"; 

            hapusInvoiceKadaluarsa(); 

            printTablePendingInvoices(); 

            bool adaInvoicePending = false;
            for (const auto& inv : invoices) {
                if (inv.status == "Pending") {
                    adaInvoicePending = true;
                    break;
                }
            }

            if (!adaInvoicePending) {
                cout << "\nTidak ada invoice yang menunggu konfirmasi saat ini.\n"; 
                system("pause");
                continue;
            }

            int idKonfirmasi;
            cout << "Masukkan ID Invoice yang akan dikonfirmasi pembayarannya (atau 0 untuk batal): ";
            while (!(cin >> idKonfirmasi)) { 
                cout << "Input ID tidak valid! Masukkan angka: "; 
                cin.clear(); 
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // bersihkan buffer

            if (idKonfirmasi == 0) {
                continue;
            }

            auto it = find_if(invoices.begin(), invoices.end(),
                            [&](const Invoice& inv) { return inv.id == idKonfirmasi && inv.status == "Pending"; }); 

            if (it == invoices.end()) { 
                cout << "Invoice dengan ID " << idKonfirmasi << " tidak ditemukan atau statusnya bukan 'Pending'.\n"; 
                system("pause"); 
                continue; 
            }

            cout << "*** Detail Invoice yang Akan Dikonfirmasi *** \n";
            printTableInvoice(*it); 

            string yn = getString("Apakah pembayaran untuk invoice ini sudah valid dan akan dikonfirmasi? (Ya/Tidak): "); 
            if (yn == "Ya" || yn == "ya" || yn == "YA") { 
                bool stokCukupUntukKonfirmasi = true;
                //cek stok sebelum konfir
                for (const auto& item : it->items) {
                    Product* p = getProduct(item.product_id);
                    if (p) {
                        if (item.qty > p->stok) {
                            cout << "GAGAL KONFIRMASI: Stok untuk produk '" << p->nama << "' (ID: " << p->id << ") tidak mencukupi.\n";
                            cout << "Dipesan: " << item.qty << ", Tersedia: " << p->stok << endl;
                            stokCukupUntukKonfirmasi = false;
                            break;
                        }
                    } else {
                        cout << "GAGAL KONFIRMASI: Produk dengan ID " << item.product_id << " dalam invoice ini tidak ditemukan di database produk." << endl;
                        stokCukupUntukKonfirmasi = false;
                        break;
                    }
                }

                if (stokCukupUntukKonfirmasi) {
                    it->status = "Berhasil"; 
                    //-stok +pembelian
                    for (const auto& item : it->items) {
                        Product* p = getProduct(item.product_id);
                        if (p) { 
                            p->stok -= item.qty;
                            p->pembelian += item.qty;
                        }
                    }
                    cout << "Invoice ID " << it->id << " berhasil dikonfirmasi! Stok produk telah diperbarui.\n"; 
                } else {
                    cout << "Invoice ID " << it->id << " TIDAK dapat dikonfirmasi karena masalah stok.\n";
                    string hapusInvStok = getString("Apakah Anda ingin menghapus invoice bermasalah ini? (Ya/Tidak): ");
                    
                    if (hapusInvStok == "Ya" || hapusInvStok == "ya") {
                        invoices.erase(it);
                        cout << "Invoice ID " << idKonfirmasi << " telah dihapus karena masalah stok.\n";
                    } else {
                        cout << "Invoice ID " << idKonfirmasi << " tetap berstatus 'Pending'. Harap periksa stok atau minta pelanggan order ulang.\n";
                    }
                }
            } else { 
                cout << "Konfirmasi pembayaran untuk Invoice ID " << it->id << " dibatalkan oleh Admin.\n"; 
                string hapusSetelahBatal = getString("Apakah Anda ingin menghapus invoice yang dibatalkan konfirmasinya ini? (Ya/Tidak): ");
                if (hapusSetelahBatal == "Ya" || hapusSetelahBatal == "ya") {
                    invoices.erase(it); 
                    cout << "Invoice ID " << idKonfirmasi << " telah dihapus.\n"; 
                } else {
                    cout << "Invoice ID " << idKonfirmasi << " tetap berstatus 'Pending'.\n";
                }
            }
            system("pause"); 

        } else if (m == 3) { //laporan penjualan bulanan
            clearScreen();
            cout << "Laporan Penjualan Bulanan ";
            int bln1, bln2, thn;
            cout << "Masukkan Tahun Laporan (ex : 2025 ): ";
            while (!(cin >> thn) || thn < 2000 || thn > 2100) {
                cout << "Input tahun tidak valid! Masukkan tahun (ex:2025) ";
                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }

            cout << "Input rentang bulan (1-12):\nDari Bulan ke-: "; 
            while (!(cin >> bln1) || bln1 < 1 || bln1 > 12) { 
                cout << "Input bulan tidak valid! Masukkan angka 1-12: "; 
                cin.clear(); 
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            }
            cout << "Sampai Bulan ke-: "; 
            while (!(cin >> bln2) || bln2 < 1 || bln2 > 12 || bln2 < bln1) { 
                cout << "Input bulan tidak valid! Masukkan angka 1-12 dan harus >= bulan 'Dari': "; 
                cin.clear(); 
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

            Table tLaporan;
            tLaporan.add_row({"ID Inv", "Username", "Total", "Status", "Tanggal", "Detail Produk"}); 
            double totalPendapatan = 0;
            bool adaData = false;

            for (const auto& inv : invoices) {
                if (inv.status != "Berhasil") continue; //hitung yang berhasil

                tm* ltm = localtime(&inv.waktu); 
                int invBulan = ltm->tm_mon + 1; 
                int invTahun = ltm->tm_year + 1900;

                if (invTahun == thn && invBulan >= bln1 && invBulan <= bln2) { 
                    adaData = true;
                    char bufferTanggal[80];
                    strftime(bufferTanggal, 80, "%Y-%m-%d", localtime(&inv.waktu));

                    string detailProdukText;
                    for (const auto& ci : inv.items) { 
                        auto p = getProduct(ci.product_id); 
                        if (p) { 
                            detailProdukText += p->nama + " (Qty: " + to_string(ci.qty) + "); "; 
                        }
                    }
                    if (!detailProdukText.empty()) detailProdukText.pop_back(), detailProdukText.pop_back(); 

                    tLaporan.add_row({to_string(inv.id), inv.username, to_string((int)inv.total), inv.status, bufferTanggal, detailProdukText}); 
                    totalPendapatan += inv.total;
                }
            }

            if (!adaData) {
                cout << "\nTidak ada data penjualan yang berhasil pada periode tersebut.\n";
            } else {
                cout << "\nLaporan Penjualan dari Bulan " << bln1 << " sampai " << bln2 << " Tahun " << thn << ":\n";
                cout << tLaporan << endl; 
                cout << "\nTotal Pendapatan pada periode ini: Rp" << fixed << setprecision(2) << totalPendapatan << endl;
            }
            system("pause"); 
        }
    }
}
void tesData() {
    users.push_back({"admin", "admin123", ADMIN, false}); //username, pass, role, status 
    products.push_back({1, "Logitech K120", "Keyboard", 10, 0, 150000});
    products.push_back({2, "Rexus Xierra", "Mouse", 15, 0, 80000});
    products.push_back({3, "Fantech MP25", "Mousepad", 20, 0, 50000});
    products.push_back({4, "Kingston 8GB", "Ram", 5, 0, 400000});
    products.push_back({5, "Cooler Master Fan", "Fan", 7, 0, 120000});
}

int main() {
    tesData();
    while (true) {
        clearScreen();
        Table mainTabel;
        tabelMainMenu(mainTabel);
        cout << mainTabel << endl;
        int m = getInputMenu(0, 2);
        if (m == 0) break;
        if (m == 2)
            registerMember();
        else if (m == 1) {
            User* u = login();
            if (!u) continue;
            if (u->role == PEMBELI)
                menuPembeli(u);
            else if (u->role == KASIR)
                menuKasir(u);
            else if (u->role == ADMIN)
                menuAdmin(u);
        }
    }
    cout << "Terima kasih!\n";
    return 0;
}
