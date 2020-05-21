#include <shell.hh>

using namespace std;

int main_func(releax::cli &app) {
    Shell shell;
    shell.loop();
    
    return 0;
}

int
main(int ac, char** av)
{
    releax::cli app;

    app.name("shell")
       .version(0.1)
       .release('a')
       .description("releax shell")
       .author("Manjeet Singh", "itsmanjeet@releax.in","-")
       .main(main_func);

    return app.execute(ac, av);

}