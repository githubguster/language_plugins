/**
 * @author guster
 */
#ifndef _PLUGIN_COLLECT_HPP_
#define _PLUGIN_COLLECT_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <typeinfo>
#include <limits.h>
#include <vector>

using namespace std;

template <class T, class Create, class Dispose>
class EverGuardPluginController
{
private:
    char *moduleName;
    char *createFunction;
    char *disposeFunction;

private:
    bool getModule(char *file, T **object)
    {
        if(this->moduleName != nullptr && this->createFunction != nullptr && this->disposeFunction != nullptr)
        {
            const char* error = nullptr;

            void *handle = dlopen(file, RTLD_LAZY);

            if (nullptr == handle)
            {
                printf("failed to open %s, error: %s", file, dlerror());
                return false;
            }

            // reset errors
            dlerror();

            Create create = (Create)dlsym(handle, this->createFunction);

            error = dlerror();
            if (error != nullptr)
            {
                printf("failed to load creator, error: %s", error);
                dlclose(handle);
                return false;
            }

            // reset errors
            dlerror();

            Dispose dispose = (Dispose)dlsym(handle, this->disposeFunction);

            error = dlerror();
            if (error != nullptr)
            {
                printf("failed to load destroyer, error: %s", error);
                dlclose(handle);
                return false;
            }

            if (!(*create)(object))
            {
                printf("failed to create object");
                dlclose(handle);
                return false;
            }

            if(strstr(typeid(*object).name(), this->moduleName) != nullptr)
            {
                //(*dispose)(object);
                //dlclose(handle);
                return true;
            }

            (*dispose)(*object);
            dlclose(handle);
            return false;
        }
        return false;
    }

    bool getClass(char *file, T **object, char *className)
    {
        if(this->moduleName != nullptr && this->createFunction != nullptr && this->disposeFunction != nullptr)
        {
            const char* error = nullptr;

            void *handle = dlopen(file, RTLD_LAZY);

            if (nullptr == handle)
            {
                error = dlerror();
                printf("failed to open %s, error: %s", file, error);
                return false;
            }

            // reset errors
            dlerror();

            Create create = (Create)dlsym(handle, this->createFunction);

            error = dlerror();
            if (error != nullptr)
            {
                printf("failed to load creator, error: %s", error);
                dlclose(handle);
                return false;
            }

            // reset errors
            dlerror();

            Dispose dispose = (Dispose)dlsym(handle, this->disposeFunction);

            error = dlerror();
            if (error != nullptr)
            {
                printf("failed to load destroyer, error: %s", error);
                dlclose(handle);
                return false;
            }

            if (!(*create)(object))
            {
                printf("failed to create object");
                dlclose(handle);
                return false;
            }

            if(strstr(typeid(*object).name(), this->moduleName) != nullptr)
            {
                if(strstr(typeid(**object).name(), className) != nullptr)
                {
                    //(*dispose)(*object);
                    //dlclose(handle);
                    return true;
                }
            }

            (*dispose)(*object);
            dlclose(handle);
            return false;
        }
        return false;
    }

public:
    EverGuardPluginController(char *moduleName, char *createFunction, char *disposeFunction)
                             :moduleName(moduleName),
                              createFunction(createFunction),
                              disposeFunction(disposeFunction)
    {
    }

    ~EverGuardPluginController()
    {
        this->moduleName = nullptr;
        this->createFunction = nullptr;
        this->disposeFunction = nullptr;
    }

    vector<T*> getModulesFromDirectory(char *dirctoryPath)
    {
        DIR *dir;
        struct dirent *dirent;
        dir = opendir(dirctoryPath);
        vector<T*> objects;

        if(dir)
        {

            while((dirent = readdir(dir)) != nullptr)
            {
                char *file = nullptr;
                T *object;

                asprintf(&file, "%s/%s", dirctoryPath, dirent->d_name);

                if(file != nullptr)
                {
                    if(this->getModulesByFileName(file, &object))
                    {
                        objects.push_back(object);
                    }

                    memset(file, 0, strlen(file));

                    free(file);
                }
                file = nullptr;
            }
            closedir (dir);
        }

        return objects;
    }

    T* getClassFromDirectory(char *dirctoryPath, char *className)
    {
        DIR *dir;
        struct dirent *dirent;
        dir = opendir(dirctoryPath);

        if(dir)
        {
            while((dirent = readdir(dir)) != nullptr)
            {
                char *file = nullptr;
                T *object;

                asprintf(&file, "%s/%s", dirctoryPath, dirent->d_name);

                if(file != nullptr)
                {
                    if(this->getClassByFileName(file, &object, className))
                    {
                        closedir (dir);
                        memset(file, 0, strlen(file));
                        free(file);
                        file = nullptr;
                        return object;
                    }

                    memset(file, 0, strlen(file));

                    free(file);
                }
                file = nullptr;
            }

            closedir (dir);
        }
        return nullptr;
    }

    bool getModulesByFileName(char *fileName, T **object)
    {
        char *dot = strrchr(fileName, '.');
        if(dot != nullptr && !strcmp(dot, ".so"))
        {
            return this->getModule(fileName, object);
        }
        else
        {
            return false;
        }
    }

    bool getClassByFileName(char *fileName, T **object, char *className)
    {
        char *dot = strrchr(fileName, '.');
        if(dot != NULL && !strcmp(dot, ".so"))
        {
            return this->getClass(fileName, object, className);
        }
        else
        {
            return false;
        }
    }
};

#endif // _PLUGIN_COLLECT_HPP_
