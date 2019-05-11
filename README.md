# SoalShift_modul4_E05
> 05111740000052 Anggar Wahyu Nur Wibowo

>  05111740000099 Bobbi Aditya

## Soal
Kita diminta untuk membuat program C yang menyerupai crontab menggunakan daemon dan thread. 

Ada sebuah file crontab.data untuk menyimpan config dari crontab. Setiap ada perubahan file tersebut maka secara otomatis program menjalankan config yang sesuai dengan perubahan tersebut tanpa perlu diberhentikan. 

Config hanya sebatas * dan 0-9 (tidak perlu /,- dan yang lainnya)

## Daemon
Dalam program cron kami, program kami akan mengecek apakah config masing-masing crontab sesuai dengan waktu sekarang. Proses pengecekan akan dilakukan setiap detik, untuk menjalankan program akan dilakukan setiap detik ke 0 jika config crontab sesuai


## Proses baca file
```c
FILE * cronconfig;
char buffer[100];
int linecnt=0, err;
cronconfig = fopen("crontab.data", "r");

if (!cronconfig)
{
    perror("Opening cronconfig");
    return (-1);
}

while (EOF != fscanf(cronconfig, "%100[^\n]\n", buffer)) {
    char *token = strtok(buffer, " ");
    int flag=0;

    strcpy(c_cmd[linecnt], "");
    while(token) {
        if(flag < 5) {
            if(!strcmp(token, "*")) c_timerules[linecnt][flag] = -1;
            else c_timerules[linecnt][flag] = atoi(token);  
        } else {
            strcat(c_cmd[linecnt], token);
            strcat(c_cmd[linecnt], " ");
        }
        token = strtok(NULL, " ");
        flag++;
    }

    linecnt++;
}
```
Untuk membaca file kita menggunakan fungsi `fscanf`. Pada saat membaca tiap line, kita mengambil masing-masing config dari crontab(ada 5 buah) menggunakan `strtok`. Config crontab akan disimpan didalam array `c_timerules`, dan perintah eksekusinya akan disimpan di dalam array `c_cmd`.

## Threading
```c
FOR(i, N) err = pthread_create(&(tid[i]), NULL, execute, NULL);

void* execute(void *arg){
    int i, f=0;
    pthread_t id = pthread_self();
    pid_t child=0;
    // printf("s\n");
    for(i=0; i<N; i++){
        // printf("%d %d %d\n", i, id, tid[i]);
        if((id == tid[i]) && (execable(c_timerules[i]))){
            printf("\n%d: %s\n", i, c_cmd[i]);
            child = fork();

            if(child == 0){
                execl("/bin/bash", "bash", "-c", c_cmd[i], NULL);
            }
        }
    }
}
```
Dalam program kami, kami menggunakan thread untuk melakukan pengecekan apakah config dari masing-masing crontab bisa dijalankan pada menit sekarang. Proses pengecekan akan dijalankan menggunakan fungsi `execable`.

Jika config crontab sesuai dengan waktu sekarang, maka perintah eksekusi yang ada di dalam array `c_cmd` akan di jalankan.

Untuk menjalankan perintah eksekusi kita menggunakan perintah `execl`

### Cek Config Crontab
```c
int execable(int *timerules){
    struct tm *tm;
    time_t tim = time(NULL);
    tm = localtime(&tim);

    int i=0, flag=1, fdom=0;
    // perhatikan 2 sama 4
    // rule 4 matikan dulu
    if(timerules[0] != tm->tm_min && timerules[0] != -1) flag = 0;
    if(timerules[1] != tm->tm_hour && timerules[1] != -1) flag = 0;
    if(timerules[2] != tm->tm_mday && timerules[2] != -1) {
        flag = 0;
        fdom = 1; // tidak sesuai tgl, tapi lihat weekday nanti
    }
    if(timerules[3] != tm->tm_mon && timerules[3] != -1) flag = 0;
    if(timerules[4] != tm->tm_wday && timerules[4] != -1) flag = 0;
    if(timerules[4] == tm->tm_wday && fdom) flag = 1; // reverse fdom effect

    if(tm->tm_sec != 0) flag = 0;

    // printf("\n");
    return flag;
}
```
Untuk mengecek apakah config corntab sesuai dengan waktu sekarang, kami menggunakan flag sebagai penanda apakah config crontab sesuai atau tidak.

Pertama kami mengecek apakah config menit, config jam, dan config tanggal apakah sesuai atau tidak dengan waktu sekarang. Pada saat mengecek tanggal, kami menambahkan flag `fdom` untuk nanti digunakan saat pengecekan hari pada minggu ini. Setelah mengecek tanggal, kita mengecek bulan. Kemudian yang terakhir kita mengecek hari pada minggu ini apakah sesuai config atau tidak, jika tanggal sekarang dan hari pada minggu ini sesuai, maka flag akan tetap menjadi `1` menandakan config crontab sesuai. 
