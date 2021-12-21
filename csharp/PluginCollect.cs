/**
 * @author guster
 */
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Plugin 
{
    public class PluginClassInformation : IDisposable
    {
        public Type type { get; set; }
        public Object module { get; set; }

        public PluginClassInformation(Type type, Object module)
        {
            this.type = type;
            this.module = module;
        }

        public void Dispose()
        {
            GC.SuppressFinalize(this);
        }

        ~PluginClassInformation()
        {
            this.Dispose();
        }
    }

    public static class PluginModules
    {
        public static readonly Object environmentalControlLock = new Object();
        public static List<PluginClassInformation> modules = new List<PluginClassInformation>();
    }

    public class PluginCollect<T> : IDisposable
    {
        private String moduleNmae;

        private T getModule(Assembly asm, String fullTypeName)
        {
            T module = default(T);

            try
            {
                Type type = asm.GetType(fullTypeName);
                if (!(type.IsNotPublic || type.IsAbstract))
                {
                    Object typeInterface = type.GetInterface(moduleNmae, true);

                    if (typeInterface != null)
                    {
                        lock (PluginModules.environmentalControlLock)
                        {
                            try
                            {
                                PluginClassInformation information = PluginModules.modules.Find(item => item.type.FullName == type.FullName);
                                if (information == null)
                                {
                                    information = new PluginClassInformation(type, (T)asm.CreateInstance(type.FullName));
                                    PluginModules.modules.Add(information);
                                }
                                module = (T)information.module;
                            }
                            catch
                            {
                            }
                        }
                    }
                }
            }
            catch
            {
            }

            return module;
        }

        private T getModule(Assembly asm, Type type)
        {
            return getModule(asm, type.FullName);
        }

        public T[] getModules(Assembly asm)
        {
            List<T> modules = new List<T>();

            try
            {
                foreach (Type type in asm.GetTypes())
                {
                    T module = getModule(asm, type);

                    if (module != null)
                    {
                        modules.Add(module);
                    }
                }
            }
            catch
            {
            }

            return modules.ToArray();
        }

        public T[] getModules(String filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                {
                    throw new FileNotFoundException();
                }
                return getModules(Assembly.LoadFile(filePath));
            }
            catch
            {
                List<T> modules = new List<T>();
                return modules.ToArray();
            }
        }

        public T[] getModulesFromDirectory(String dirctoryPath)
        {
            List<T> modules = new List<T>();

            foreach (String filePath in Directory.GetFiles(dirctoryPath, "*.*", SearchOption.TopDirectoryOnly).Where(s => s.EndsWith(".dll") || s.EndsWith(".exe")))
            {
                modules.AddRange(getModules(filePath));
            }

            return modules.ToArray();
        }

        public PluginCollect(String moduleNmae)
        {
            this.moduleNmae = moduleNmae;
        }

        ~PluginCollect()
        {
            this.Dispose();
        }

        public void Dispose()
        {
            GC.SuppressFinalize(this);
        }
    }
}