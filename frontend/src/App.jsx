import React, { useState, useRef, useEffect } from 'react';
import { Terminal as TerminalIcon, Play, ChevronRight, Server, ShieldAlert, User, Key, Database, X, CheckCircle, LogOut, HardDrive, Folder, FileText, ChevronLeft, Hexagon, LayoutGrid } from 'lucide-react';
import axios from 'axios';

// ==================== MOCK DATA PARA VISUALIZADOR ====================
const P_MOCK_DISKS = [
  { id: 'd1', name: 'Disco_Primario.dk', capacity: '50MB', fit: 'Best Fit', date: '26/09/2026', partitions: 4 },
  { id: 'd2', name: 'Respaldo_Sistema.dk', capacity: '120MB', fit: 'First Fit', date: '26/09/2026', partitions: 2 }
];

const P_MOCK_PARTS = [
  { id: 'p1', name: 'Particion1', type: 'Primaria', status: 'Activa (Montada)', size: '20MB', fit: 'Worst Fit' },
  { id: 'p2', name: 'Particion2_EXT3', type: 'Extendida', status: 'Inactiva', size: '30MB', fit: 'First Fit' }
];

const P_MOCK_FS = [
  { name: 'bin', type: 'folder', perms: '777' },
  { name: 'home', type: 'folder', perms: '664' },
  { name: 'var', type: 'folder', perms: '664' },
  { name: 'users.txt', type: 'file', perms: '664', content: "1,G,root\n1,U,root,root,123\n" },
  { name: 'readme.md', type: 'file', perms: '777', content: "# Sistema Inicializado\nBienvenidos a EXT3." }
];

const P_MOCK_HOME_FS = [
  { name: 'documentos', type: 'folder', perms: '664' },
  { name: 'imagenes', type: 'folder', perms: '777' },
  { name: 'tareas_mia.txt', type: 'file', perms: '644', content: "Manejo e Implementación de Archivos - Proyecto 2" }
];
// =====================================================================


function App() {
  const [outputLines, setOutputLines] = useState([
    { text: 'Bienvenido a C++ DISK ONLINE [Versión 2.0]', type: 'system' },
    { text: 'Sistema de Archivos EXT2/EXT3 listo para operar.', type: 'system' }
  ]);
  const [currentInput, setCurrentInput] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  
  // States: Login & Navigation
  const [showLoginModal, setShowLoginModal] = useState(false);
  const [loginForm, setLoginForm] = useState({ user: '', password: '', id: '' });
  const [loggedInUser, setLoggedInUser] = useState(null);
  
  // State: Tab & Visualizador
  const [activeTab, setActiveTab] = useState('terminal'); // 'terminal' | 'disks' | 'partitions' | 'explorer'
  const [selectedDisk, setSelectedDisk] = useState(null);
  const [selectedPartition, setSelectedPartition] = useState(null);
  const [currentPath, setCurrentPath] = useState('/');
  const [currentFileSystem, setCurrentFileSystem] = useState(P_MOCK_FS);
  const [viewingFileContent, setViewingFileContent] = useState(null);

  const endOfTerminalRef = useRef(null);
  useEffect(() => {
    if (activeTab === 'terminal') {
      endOfTerminalRef.current?.scrollIntoView({ behavior: 'smooth' });
    }
  }, [outputLines, activeTab]);

  const handleCommandSubmit = async (e) => {
    e.preventDefault();
    if (!currentInput.trim()) return;
    const cmd = currentInput.trim();
    setOutputLines(prev => [...prev, { text: `user@sys:~$ ${cmd}`, type: 'command' }]);
    setCurrentInput('');
    setIsProcessing(true);

    try {
      const response = await axios.post('http://localhost:8080/api/analizar', cmd, { headers: { 'Content-Type': 'text/plain' } });
      const serverText = response.data || 'Ejecutado (Sin salida)';
      const lineas = serverText.split('\n').filter(l => l.trim() !== '');
      const newLines = lineas.map(line => ({
        text: line,
        type: line.toLowerCase().includes('error') ? 'error' : 'output'
      }));
      setOutputLines(prev => [...prev, ...newLines]);
    } catch (error) {
      setOutputLines(prev => [...prev, { text: 'Terminal: C++ connection refused.', type: 'error' }]);
    }
    setIsProcessing(false);
  };

  const handleLoginSubmit = async (e) => {
    e.preventDefault();
    setIsProcessing(true);
    const cmd = `login -user=${loginForm.user} -pass=${loginForm.password} -id=${loginForm.id}`;
    try {
      const response = await axios.post('http://localhost:8080/api/analizar', cmd, { headers: { 'Content-Type': 'text/plain' } });
      if (response.data.toLowerCase().includes('error')) {
        setOutputLines(prev => [...prev, { text: response.data, type: 'error' }]);
      } else {
        setOutputLines(prev => [...prev, { text: `✅ Sesión iniciada exitosamente: ${loginForm.user}`, type: 'success' }]);
        setLoggedInUser(loginForm.user);
        setShowLoginModal(false);
        setActiveTab('disks'); // Transición cool hacia el visualizador al loguearse
      }
    } catch (error) {
      setOutputLines(prev => [...prev, { text: 'Terminal: Error al intentar loguearse (Fallo de red).', type: 'error' }]);
    }
    setIsProcessing(false);
  };

  const enterFolder = (folderName) => {
    if (currentPath === '/') {
      setCurrentPath('/' + folderName);
      setCurrentFileSystem(folderName === 'home' ? P_MOCK_HOME_FS : []);
    }
  };

  const goBackFolder = () => {
    setCurrentPath('/');
    setCurrentFileSystem(P_MOCK_FS);
  };

  return (
    <div className="min-h-screen bg-slate-950 flex flex-col items-center justify-start md:justify-center pt-8 md:pt-0 p-4 lg:p-8 bg-[url('https://www.transparenttextures.com/patterns/cubes.png')] relative overflow-y-auto overflow-x-hidden">
      
      {/* Background glow effects */}
      <div className="absolute top-0 left-1/4 w-96 h-96 bg-brand-600/20 rounded-full blur-3xl -translate-y-1/2 pointer-events-none"></div>
      <div className="absolute bottom-0 right-1/4 w-96 h-96 bg-cyan-600/10 rounded-full blur-3xl translate-y-1/2 pointer-events-none"></div>

      <div className="w-full max-w-5xl glass-panel rounded-2xl flex flex-col h-[85vh] relative z-10 transition-all duration-300 hover:shadow-brand-500/5 mb-8">
        
        {/* Header Bar */}
        <div className="h-14 border-b border-white/10 flex items-center justify-between px-6 shrink-0">
          <div className="flex items-center gap-3">
            <TerminalIcon className="w-5 h-5 text-brand-400" />
            <h1 className="font-semibold text-slate-200 tracking-wide">C++ DISK ONLINE</h1>
          </div>
          
          <div className="flex items-center gap-4">
            {loggedInUser ? (
              <div className="flex items-center gap-3">
                <span className="text-xs text-brand-300 bg-brand-500/20 px-3 py-1 rounded-full ring-1 ring-brand-500/30">
                  User: <strong>{loggedInUser}</strong>
                </span>
                <button onClick={() => { setLoggedInUser(null); setActiveTab('terminal'); }} className="text-red-400 hover:text-red-300 transition-colors text-sm flex items-center gap-1 bg-red-400/10 px-2 py-1 rounded-md">
                  <LogOut className="w-4 h-4" /> Salir
                </button>
              </div>
            ) : (
              <button onClick={() => setShowLoginModal(true)} className="text-slate-200 bg-brand-600 hover:bg-brand-500 transition-colors text-sm flex items-center gap-2 px-4 py-1.5 rounded-md shadow-[0_0_15px_rgba(79,70,229,0.4)]">
                <ShieldAlert className="w-4 h-4" /> <span>Iniciar Sesión Gráfica</span>
              </button>
            )}
            <div className="flex gap-2 ml-4">
              <div className="w-3 h-3 rounded-full bg-slate-700 hover:bg-red-500 transition-colors cursor-pointer"></div>
              <div className="w-3 h-3 rounded-full bg-slate-700 hover:bg-yellow-500 transition-colors cursor-pointer"></div>
              <div className="w-3 h-3 rounded-full bg-slate-700 hover:bg-green-500 transition-colors cursor-pointer"></div>
            </div>
          </div>
        </div>

        {/* Main Interface */}
        <div className="flex flex-1 overflow-hidden">
          
          {/* Sidebar */}
          <div className="w-64 border-r border-white/5 p-4 hidden md:flex flex-col gap-6 shrink-0 bg-slate-900/20">
            <div>
              <h3 className="text-xs font-semibold text-slate-500 uppercase tracking-wider mb-3">Vistas</h3>
              <div className="space-y-2">
                <button onClick={() => setActiveTab('terminal')} className={`w-full flex items-center gap-3 px-3 py-2 rounded-lg text-sm transition-all group ${activeTab === 'terminal' ? 'bg-brand-500/20 border-brand-500/50 text-brand-300 border' : 'bg-transparent border-transparent text-slate-400 hover:bg-white/5 border'}`}>
                  <TerminalIcon className={`w-4 h-4 ${activeTab === 'terminal' ? 'text-brand-400' : 'text-slate-500 group-hover:text-brand-400'}`} />
                  Terminal CLI
                </button>
                <button onClick={() => loggedInUser ? setActiveTab('disks') : alert('Inicie sesión primero')} className={`w-full flex items-center gap-3 px-3 py-2 rounded-lg text-sm transition-all group ${activeTab !== 'terminal' ? 'bg-brand-500/20 border-brand-500/50 text-brand-300 border' : 'bg-transparent border-transparent text-slate-400 hover:bg-white/5 border'} ${!loggedInUser ? 'opacity-40 cursor-not-allowed' : ''}`}>
                  <LayoutGrid className={`w-4 h-4 ${activeTab !== 'terminal' ? 'text-brand-400' : 'text-slate-500 group-hover:text-brand-400'}`} />
                  Explorador Visual
                </button>
              </div>
            </div>
          </div>

          <div className="flex-1 flex flex-col bg-[#050510] relative">
            
            {/* ===================== TAB TERMINAL ===================== */}
            {activeTab === 'terminal' && (
              <React.Fragment>
                <div className="flex-1 overflow-y-auto p-6 space-y-2 terminal-scroll font-mono text-[13px] md:text-sm">
                  {outputLines.map((line, idx) => (
                    <div key={idx} className="flex font-mono">
                      {line.type === 'command' && ( <span className="text-brand-400 mr-3 shrink-0">{'❯'}</span> )}
                      <span className={`break-all ${
                        line.type === 'command' ? 'text-blue-300' : line.type === 'error' ? 'text-red-400' : line.type === 'success' ? 'text-green-400' : line.type === 'info' ? 'text-amber-300' : 'text-slate-300'
                      }`}>{line.text}</span>
                    </div>
                  ))}
                  <div ref={endOfTerminalRef}></div>
                </div>

                <form onSubmit={handleCommandSubmit} className="p-4 border-t border-white/10 shrink-0 bg-[#070715]">
                  <div className="relative flex items-center">
                    <ChevronRight className="absolute left-3 w-5 h-5 text-brand-500" />
                    <input type="text" value={currentInput} onChange={(e) => setCurrentInput(e.target.value)} placeholder="Ejecutar comando (ej. rep -id=36a1)..." className="w-full bg-slate-900/50 border border-slate-700/50 rounded-lg h-12 pl-10 pr-4 text-slate-200 font-mono text-sm focus:outline-none focus:border-brand-500/50 transition-all shadow-inner" spellCheck="false" disabled={isProcessing} />
                  </div>
                </form>
              </React.Fragment>
            )}

            {/* ===================== TAB DISCOS ===================== */}
            {activeTab === 'disks' && (
              <div className="p-8 flex-1 overflow-y-auto animate-in fade-in zoom-in-95 duration-300">
                <h2 className="text-xl font-bold text-white mb-6 flex items-center gap-2"> <Server className="text-brand-400"/> Selección de Disco Principal </h2>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                  {P_MOCK_DISKS.map(disk => (
                    <div key={disk.id} onClick={() => { setSelectedDisk(disk); setActiveTab('partitions'); }} className="bg-slate-900/80 border border-slate-700 hover:border-brand-500/60 p-6 rounded-xl cursor-pointer hover:-translate-y-1 transition-all group shadow-lg hover:shadow-brand-500/20">
                      <div className="flex gap-4">
                        <div className="w-14 h-14 rounded-full bg-brand-500/10 flex items-center justify-center group-hover:bg-brand-500/20 transition-colors">
                          <HardDrive className="w-7 h-7 text-brand-400" />
                        </div>
                        <div>
                          <h3 className="text-lg font-semibold text-slate-200 group-hover:text-brand-300 transition-colors">{disk.name}</h3>
                          <p className="text-sm text-slate-400 mt-1 flex gap-3">
                            <span className="bg-slate-800 px-2 rounded">{disk.capacity}</span>
                            <span className="bg-slate-800 px-2 rounded text-xs">{disk.fit}</span>
                          </p>
                          <p className="text-xs text-slate-500 mt-3 pt-3 border-t border-slate-700/50 flex justify-between">
                            <span>Particiones: {disk.partitions}</span>
                            <span>Creado: {disk.date}</span>
                          </p>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              </div>
            )}

            {/* ===================== TAB PARTICIONES ===================== */}
            {activeTab === 'partitions' && (
              <div className="p-8 flex-1 overflow-y-auto animate-in slide-in-from-right-4 duration-300">
                <div className="flex items-center gap-4 mb-6">
                  <button onClick={() => setActiveTab('disks')} className="p-2 hover:bg-slate-800 rounded-lg text-slate-400 hover:text-white transition-colors"><ChevronLeft/></button>
                  <h2 className="text-xl font-bold text-white flex items-center gap-2"> <Hexagon className="text-purple-400"/> Particiones en {selectedDisk?.name} </h2>
                </div>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                  {P_MOCK_PARTS.map(part => (
                    <div key={part.id} onClick={() => { setSelectedPartition(part); setActiveTab('explorer'); }} className="bg-slate-900/80 border border-slate-700 hover:border-purple-500/60 p-5 rounded-xl cursor-pointer hover:bg-slate-800/80 transition-all flex items-center justify-between">
                      <div>
                        <h3 className="font-medium text-slate-200 flex items-center gap-2">
                           <div className={`w-2.5 h-2.5 rounded-full ${part.status.includes('Activa') ? 'bg-green-500' : 'bg-red-500'} shadow-[0_0_8px_rgba(34,197,94,0.6)]`}></div>
                           {part.name}
                        </h3>
                        <p className="text-sm text-slate-400 mt-1">
                          {part.type} | Tamaño: {part.size}
                        </p>
                      </div>
                      <ChevronRight className="text-slate-600" />
                    </div>
                  ))}
                </div>
              </div>
            )}

            {/* ===================== TAB EXPLORADOR (FILE SYSTEM) ===================== */}
            {activeTab === 'explorer' && (
              <div className="flex-1 flex flex-col animate-in slide-in-from-right-4 duration-300">
                <div className="h-16 px-6 border-b border-white/5 flex items-center gap-4 bg-slate-950/40">
                  <button onClick={() => setActiveTab('partitions')} className="p-2 hover:bg-slate-900 rounded-lg text-slate-400 hover:text-white"><ChevronLeft/></button>
                  <div className="h-6 w-px bg-slate-800"></div>
                  <div className="flex items-center gap-2 overflow-x-auto text-sm text-slate-300 font-mono">
                    <span className="text-brand-400 opacity-60">root@ext3</span>
                    <span className="text-slate-600">:</span>
                    <span className="text-blue-300 bg-blue-500/10 px-2 py-0.5 rounded cursor-pointer hover:bg-blue-500/20" onClick={goBackFolder}>{currentPath}</span>
                  </div>
                </div>
                <div className="p-6 flex-1 overflow-y-auto">
                  <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-4">
                    {currentPath !== '/' && (
                      <div onClick={goBackFolder} className="flex flex-col items-center gap-3 p-4 rounded-xl hover:bg-slate-900/60 cursor-pointer group transition-all text-center border border-transparent hover:border-slate-800">
                        <Folder className="w-12 h-12 text-slate-600 group-hover:text-amber-500 transition-colors" fill="currentColor" fillOpacity="0.2" />
                        <span className="text-sm font-medium text-slate-300 group-hover:text-white">.. (Atrás)</span>
                      </div>
                    )}
                    {currentFileSystem.map((item, i) => (
                      <div key={i} onClick={() => { if(item.type==='folder') enterFolder(item.name); else setViewingFileContent(item); }} className="flex flex-col items-center gap-3 p-4 rounded-xl hover:bg-slate-900/60 cursor-pointer group transition-all text-center border border-transparent hover:border-slate-800">
                        {item.type === 'folder' ? (
                          <Folder className="w-12 h-12 text-blue-500/80 group-hover:text-brand-400 transition-colors" fill="currentColor" fillOpacity="0.2" />
                        ) : (
                          <FileText className="w-12 h-12 text-slate-400 group-hover:text-white transition-colors" />
                        )}
                        <div>
                          <p className="text-sm font-medium text-slate-300 group-hover:text-white truncate w-24">{item.name}</p>
                          <p className="text-[10px] text-slate-500 font-mono mt-1">Perm: {item.perms}</p>
                        </div>
                      </div>
                    ))}
                    {currentFileSystem.length === 0 && ( <div className="col-span-full py-10 text-center text-slate-500">Carpeta vacía.</div> )}
                  </div>
                </div>
              </div>
            )}

          </div>
        </div>
      </div>

      {/* FILE CONTENT VIEWER MODAL */}
      {viewingFileContent && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/70 backdrop-blur-sm animate-in fade-in duration-200">
          <div className="bg-slate-950 border border-slate-700/50 rounded-xl shadow-[0_0_50px_rgba(0,0,0,0.5)] w-full max-w-2xl overflow-hidden flex flex-col h-[70vh]">
            <div className="h-12 border-b border-slate-800 flex items-center justify-between px-4 bg-slate-900">
              <span className="text-brand-300 font-mono text-sm flex items-center gap-2"><FileText className="w-4 h-4"/> {viewingFileContent.name}</span>
              <button onClick={() => setViewingFileContent(null)} className="text-slate-400 hover:text-white"><X className="w-5 h-5"/></button>
            </div>
            <div className="flex-1 overflow-auto p-4 bg-[#0a0a0f]">
              <pre className="text-slate-300 font-mono text-sm leading-relaxed">{viewingFileContent.content}</pre>
            </div>
          </div>
        </div>
      )}

      {/* LOGIN MODAL OVERLAY */}
      {/* ... [Ya configurado en la fase anterior] ... */}
      {showLoginModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-sm animate-in fade-in duration-200">
          {/* Mismo codigo para Formulario de validacion JSX omitido su re-copia por concision y enfocado en logic */}
          <div className="bg-slate-900 border border-slate-700 rounded-xl shadow-2xl w-full max-w-md overflow-hidden p-6 text-center">
              <h2 className="text-lg font-semibold text-white mb-4">Ingreso Rápido</h2>
              <form onSubmit={handleLoginSubmit} className="space-y-4">
                <input type="text" placeholder="Usuario" required value={loginForm.user} onChange={e => setLoginForm({...loginForm, user:e.target.value})} className="w-full bg-slate-950 border border-slate-700 rounded-lg p-3 text-slate-200 focus:border-brand-500 outline-none" />
                <input type="password" placeholder="Contraseña" required value={loginForm.password} onChange={e => setLoginForm({...loginForm, password:e.target.value})} className="w-full bg-slate-950 border border-slate-700 rounded-lg p-3 text-slate-200 focus:border-brand-500 outline-none" />
                <input type="text" placeholder="ID Particion" required value={loginForm.id} onChange={e => setLoginForm({...loginForm, id:e.target.value})} className="w-full bg-slate-950 border border-slate-700 rounded-lg p-3 text-slate-200 focus:border-brand-500 outline-none uppercase" />
                <button type="submit" className="w-full bg-brand-600 hover:bg-brand-500 text-white font-medium py-3 rounded-lg transition-colors mt-2">Iniciar Sesión</button>
                <button onClick={() => setShowLoginModal(false)} type="button" className="w-full text-slate-400 hover:text-white underline mt-2 text-sm">Cancelar</button>
              </form>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;
