import os, sys, string, re
import configure, rsfdoc

if os.environ.has_key('RSFROOT'):
    root = os.environ['RSFROOT']
else:
    root = os.getcwd()

bindir = os.path.join(root,'bin')
libdir = os.path.join(root,'lib')
incdir = os.path.join(root,'include')

env = Environment()

##########################################################################
# CONFIGURATION
##########################################################################

opts = Options('config.py')
configure.options(opts)
opts.Add('RSFROOT','RSF installation root',root)
opts.Update(env)

if not os.path.isfile('config.py'):
    conf = Configure(env,custom_tests={'CheckAll':configure.check_all})
    conf.CheckAll()
    env = conf.Finish()
    
Help(opts.GenerateHelpText(env))
opts.Save('config.py',env)

config = env.Command('config.py','configure.py',"")
env.Precious(config)
env.InstallAs(os.path.join(libdir,'rsfconfig.py'),'config.py')
env.InstallAs(os.path.join(libdir,'rsfconf.py'),'configure.py')
Clean(config,['#/config.log','#/.sconf_temp','configure.pyc'])
env.Alias('config',config)

##########################################################################
# SELF DOCUMENTATION
##########################################################################
Doc = Builder (action = Action(rsfdoc.selfdoc,varlist=['rsfprefix']),
               src_suffix='.c',suffix='.py')
env.Append(BUILDERS = {'Doc' : Doc})

##########################################################################
# FILT BUILD
##########################################################################
env.Append(CPPPATH=['../../include'],
           LIBPATH=['../../filt/lib'],
           LIBS=['rsf','m'])

if sys.platform[:5] == 'sunos':
    env.Append(LIBS=['nsl'])
    env['CCFLAGS']='-xO2'

Export('env')
dirs = ('lib','main','proc','imag')

Default('build/include')
for dir in map(lambda x: os.path.join('filt',x), dirs):
    build = os.path.join('build',dir)
    BuildDir(build,dir)
    SConscript(dirs=build,name='SConstruct')
    Default(build)

if env.has_key('F90'):
    modsuffix = env.get('F90MODSUFFIX')
    if modsuffix:
        env.Install(incdir,'RSF'+modsuffix)
        Clean(build,'RSF'+modsuffix)
    elif re.search(r'ifc$',env.get('F90')): # old Intel compiler quirks
        env.InstallAs(os.path.join(incdir,'rsf.pc'),'work.pc')
        env.Install(incdir,'rsf.d')
        Clean(build,['rsf.d','work.pc'])

##########################################################################
# PLOT BUILD
##########################################################################
env.Prepend(LIBPATH=['../../plot/lib'],LIBS=['rsfplot'])

Export('env')
pdirs = ('lib','main','test')

Default('build/include')
for dir in map(lambda x: os.path.join('plot',x), pdirs):
    build = os.path.join('build',dir)
    BuildDir(build,dir)
    SConscript(dirs=build,name='SConstruct')
    Default(build)

##########################################################################
# PYTHON MODULES
##########################################################################

for src in ('doc','proj','prog'):
    py = "rsf%s.py"% src
    pyc = py + 'c'
    env.Install(libdir,py)
    Clean(os.path.join(libdir,py),[os.path.join(libdir,pyc),pyc])
env.Install(bindir,'sfdoc')
env.Install(bindir,'sftour')

use = os.path.join(libdir,'rsfuse.py')
env.Command(use,None,action=Action(rsfdoc.use))
Depends(use,map(lambda x: os.path.join(libdir,'sf'+x+'.py'),dirs[1:]))
Depends(use,os.path.join(libdir,'sfplot.py'))
Depends(use,os.path.join(libdir,'vpplot.py'))
AlwaysBuild(use)

##########################################################################
# INSTALLATION
##########################################################################

env.Alias('install',[bindir,libdir,incdir])

# 	$Id: SConstruct,v 1.26 2004/04/02 15:46:41 fomels Exp $	
