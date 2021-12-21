<?php
/**
 * @author guster
 */
namespace APP\Libraries\Plugins\Collects;

use RecursiveDirectoryIterator;
use RecursiveIteratorIterator;
use ReflectionClass;
use RegexIterator;
use App\Libraries\Plugins\Collects\PluginClassInformation;

class PluginCollect
{
    private $path;
    private $moduleName;
    private $modules;
    private $informations;

    /**
     * get class informatoin
     * @return PluginClassInformation array
     */
    private function findClass()
    {
        if(isset($this->path) && !empty($this->path))
        {
            if(is_dir($this->path))
            {
                $files = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($this->path));
                $phpFiles = new RegexIterator($files, '/\.php$/');

                foreach ($phpFiles as $phpFile)
                {
                    $content = file_get_contents($phpFile->getRealPath());
                    $tokens = token_get_all($content);
                    $namespace = '';
                    
                    for($index = 0; isset($tokens[$index]); $index++)
                    {
                        if(!isset($tokens[$index][0]))
                        {
                            continue;
                        }
                        else if($tokens[$index][0] === T_NAMESPACE)
                        {
                            $index += 2; //skip namespace and whitespace
                            while(isset($tokens[$index]) && is_array($tokens[$index]))
                            {
                                if($tokens[$index][0] === T_NS_SEPARATOR || $tokens[$index][0] === T_STRING) 
                                {
                                    $namespace .= $tokens[$index][1];
                                }
                                $index++;
                            }
                        }
                        else if($tokens[$index][0] === T_CLASS)
                        {
                            $index += 2; //skip class and whitespace

                            array_push($this->informations, new PluginClassInformation($phpFile, $namespace, $tokens[$index][1]));
                        }
                    }
                }
            }
        }
    }

    /**
     * create module
     */
    private function createModules()
    {
        if(isset($this->informations) && is_array($this->informations))
        {
            foreach($this->informations as $information)
            {
                if(!class_exists($information->fullClass, false))
                {
                    require_once($information->path);
                }
                $reflectClass = new ReflectionClass($information->fullClass);
                if($reflectClass->implementsInterface($this->moduleName))
                {
                    if(class_exists($information->fullClass) && is_string($information->fullClass))
                    {
                        $class = $information->fullClass;
                        $module = new $class();
                        array_push($this->modules, $module);
                    }
                }
                unset($reflectClass);
            }
        }
    }

    /**
     * @param string $path
     * @param string $moduleName
     */
    public function __construct(string $path, string $moduleName)
    {
        $this->path = $path;
        $this->moduleName = $moduleName;
        $this->modules = array();
        $this->informations = array();

        $this->findClass();
        $this->createModules();
    }

    public function __destruct()
    {
        if(isset($this->path))
        {
            unset($this->path);
        }
        if(isset($this->moduleName))
        {
            unset($this->moduleName);
        }
        if(isset($this->modules))
        {
            unset($this->modules);
        }
        if(isset($this->informations))
        {
            unset($this->informations);
        }
    }

    public function getModules()
    {
        return $this->modules;
    }
}