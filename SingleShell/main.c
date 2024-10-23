/*  31/03/2024
    Mirza Etka Topal   22120205033
    Samet Elmalı       22120205060
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_CMD 256
#define MAX_ARGS 64
#define MAX_PATH 128

void readUsrCmd(char *cmdBuffer, int maxSize);
void loglist(char *command);
int tokenize(char *str, char *tokens[], int maxtoken);

void readUsrCmd(char *cmdBuffer, int maxSize)
{
    int r = read(STDIN_FILENO, cmdBuffer, maxSize - 1);
    if (r < 0)
    {
        perror("Okuma hatası");
        exit(EXIT_FAILURE);
    }

    /*if (r == 0)
    {
        perror("Kullanıcı girişi yok");
        exit(EXIT_FAILURE);
    }*/

    /*Okunan girdinin sonunda yeni satır karakteri varsa '\n', onu sil*/
    if (cmdBuffer[r - 1] == '\n')
    {
        cmdBuffer[r - 1] = '\0';
    }
    else
    {
        cmdBuffer[r] = '\0';
    }
}

/*log dosyası oluşturma ve girilen komutları log dosyasına yazma*/
void loglist(char *command)
{
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("log.txt dosyası açılamadı");
        exit(EXIT_FAILURE);
    }

    /*zaman değişkinlerini alma*/
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t curtime = tv.tv_sec;
    struct tm *timeinfo = localtime(&curtime);

    /*zaman bilgilerini ve komut bilgilerini buffera yazma*/
    char buffer[MAX_CMD];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d\t %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, command);

    /*bufferı log dosyasına yazma*/
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written < 0)
    {
        perror("log dosyasına yazılamadı");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

/*chatgpt den alınmıştır*/
int tokenize(char *str, char *tokens[], int maxtoken)
{
    const char *delim = "\t \n"; /* Ayraç karakterleri: sekme, boşluk, yeni satır*/
    char *token = strtok(str, delim);
    int i = 0;/*token indexi*/
    int in_quote = 0;/*" " işareti içinde mi kontrol flagi*/
    char temp[256] = "";/*geçici depolama için string*/

    
    while (token != NULL && i < maxtoken - 1)
    {
        /*Eğer başlangıçta bir tırnak işareti varsa tırnak kontrol değişkenini 1 yapılır*/
        if (token[0] == '"' && !in_quote)
        {
            in_quote = 1; 
            strcpy(temp, token + 1);
        }
        /* (" ") işareti içindeysek*/
        else if (in_quote)
        {
            strcat(temp, " ");
            strcat(temp, token); /*Tokenı geçici diziye ekle*/

            /* Eğer tokenın sonunda "" işareti varsa*/
            if (token[strlen(token) - 1] == '"')
            {
                /*tırnak kontrolü 0 yapılır*/
                in_quote = 0;
                temp[strlen(temp) - 1] = '\0'; /* Son alıntı işaretini kaldırma*/
                tokens[i] = strdup(temp); /* tokens dizisine parçayı ekleme*/
                i++; 
                temp[0] = '\0'; 
            }
        }
        /* ("") içinde değilsek tab ve boşluk karakterlerine göre kodu parçalama*/
        else
        {
            tokens[i] = strdup(token); 
            i++; 
        }
        token = strtok(NULL, delim); /*Bir sonraki parçayı al*/
    }
    tokens[i] = NULL; /*tokens dizisini NULL ile sonlandır*/
    return i;
}

/*ilk exec fonksiyonu çalışmayınca bu fonksiyon çalışır*/
int execute_command(char *command, char *args[])
{

    char errorMsg[MAX_CMD] = {0};
    char path[MAX_PATH] = {0};
    char currentPath[MAX_PATH] = {0};
    sprintf(errorMsg, "%s: command not found\n", command);

    int pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        /*komutunu pathini almak için path.txt dosyasını açma*/
        int f = open("path.txt", O_TRUNC | O_RDWR | O_CREAT | O_APPEND, 0644);
        if (f < 0)
        {
            perror("dosya açılamadı.\n");
            exit(EXIT_FAILURE);
        }

        /* fdOld da standart out tutma */
        int fdOld = dup(fileno(stdout)); /* bu kısım gerekli değil*/
        if (fdOld == -1)
        {
            perror("dup");
            exit(EXIT_FAILURE);
        }

        /*standart out u f dosyasına yönlendirme*/
        if (dup2(f, fileno(stdout)) == -1)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        /*which ile komutun yerini bulma*/
        /*which komutunun çıktısı path.txt dosyasına yazılır*/
        execlp("which", "which", command, NULL);

        /*hata olması durumunda hata mesajını yazdırmak için stdoutu eski haline getirme*/
        if (dup2(fdOld, fileno(stdout)) == -1)
        {
            perror("stdout eski haline getirilemedi");
            exit(EXIT_FAILURE);
        }

        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else
    {

        int status;
        waitpid(pid, &status, 0);

        /*child procces de oluşturulan dosyayı okuma modunda açma*/
        int f = open("path.txt", O_RDONLY);
        if (f < 0)
        {
            perror("dosya açılamadı.\n");
            exit(EXIT_FAILURE);
        }

        int r = read(f, path, MAX_PATH);
        close(f);

        if (r < 0)
        {
            perror("okuma hatası.\n");
            exit(EXIT_FAILURE);
        }

        /*eğer komutun dosya yolu which ile bulunamadıysa current pathte olup olmadığını kontrol eder*/
        if (r == 0)
        {
            /*X_OK çalıştırma izni*/
            if (access(command, X_OK) == 0)
            {
                /*Eğer komut mevcutsa, yerel olarak çalıştır*/
                strcat(currentPath, "./");
                strcat(currentPath, command);
                execvp(currentPath, args);
                perror("current pathdeki komut çalıştırılamadı");
                exit(EXIT_FAILURE);
            }
            /*F_OK dosya var mı yokmu modu*/
            if (access(command, F_OK) == 0)
            {
                execvp(command, args);
                perror("bu isimde çalıştırılabilir dosya yok");
                exit(EXIT_FAILURE);
            }

            /*komut current path değil ve path.txt dosyası da boş ise komut olmadığı için
            hata mesajı yazdırma*/
            write(1, errorMsg, strlen(errorMsg));
            exit(EXIT_FAILURE);
        }

        if (path[strlen(path) - 1] == '\n')
        {
            path[strlen(path) - 1] = '\0';
        }

        /*path.txt deki pathteki komutu çalıştırma*/
        int ee = execv(path, args);

        if (ee < 0)
        {
            write(1, errorMsg, strlen(errorMsg));
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

int main()
{
    char startingChar = '$';
    char userCmd[MAX_CMD] = {0};
    char *args[MAX_ARGS] = {0};
    int token_nums = 0;
    int i=0;

    while (1)
    {
        write(STDOUT_FILENO, &startingChar, 1);
        readUsrCmd(userCmd, MAX_CMD);

        userCmd[strcspn(userCmd, "\n")] = '\0';
        if (strcmp(userCmd, "exit") == 0)
        {
            loglist(userCmd);
            exit(EXIT_SUCCESS);
        }

        loglist(userCmd);

        token_nums = tokenize(userCmd, args, MAX_ARGS);

        pid_t pid = fork();

        if (pid == -1)
        {
            perror("Fork hatası");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execvp(args[0], args);
            /*execvp hata verdiyse execute_command fonksiyonunu kullanarak işlemi gerçekleştir*/
            execute_command(args[0], args);
        }
        else
        {      
            wait(NULL);/* chil processin bitmesini bekler*/
            
        }
    }
    return 0;
}