<?php
/**
 * @author guster
 */
namespace APP\Libraries\Plugins\Collects;

class PluginClassInformation
{
    private $path;
    private $namespace;
    private $class;
    private $fullClass;

    /**
     * @param string $path
     * @param string $namespace
     * @param string $class
     */
    public function __construct(string $path, string $namespace, string $class)
    {
        $this->path = $path;
        $this->namespace = $namespace;
        $this->class = $class;
        $this->fullClass = $namespace . '\\' . $class;
    }

    public function __destruct()
    {
        if(isset($this->path))
        {
            unset($this->path);
        }
        if(isset($this->namespace))
        {
            unset($this->namespace);
        }
        if(isset($this->class))
        {
            unset($this->class);
        }
        if(isset($this->fullClass))
        {
            unset($this->fullClass);
        }
    }

    public function __get($name)
    {
        return $this->$name;
    }
}