// The app shell: the sidebar on the left and the selected page on the right.
// On a phone the sidebar is hidden behind a menu button.
import { useState } from 'react';
import {
  ArrowLeftRight, CalendarClock, LayoutDashboard, Menu, PiggyBank, Target, TrendingUp, Wallet, X,
} from 'lucide-react';
import { ToastProvider } from './components/Toast.jsx';
import Dashboard from './pages/Dashboard.jsx';
import Transactions from './pages/Transactions.jsx';
import Budgets from './pages/Budgets.jsx';
import Goals from './pages/Goals.jsx';
import Investments from './pages/Investments.jsx';
import Pending from './pages/Pending.jsx';

const PAGES = [
  { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard, Page: Dashboard },
  { id: 'transactions', label: 'Transactions', icon: ArrowLeftRight, Page: Transactions },
  { id: 'budgets', label: 'Budgets', icon: PiggyBank, Page: Budgets },
  { id: 'goals', label: 'Goals', icon: Target, Page: Goals },
  { id: 'investments', label: 'Investments', icon: TrendingUp, Page: Investments },
  { id: 'pending', label: 'Bills', icon: CalendarClock, Page: Pending },
];

export default function App() {
  const [pageId, setPageId] = useState('dashboard');
  const [menuOpen, setMenuOpen] = useState(false);
  const current = PAGES.find(p => p.id === pageId);

  const open = id => {
    setPageId(id);
    setMenuOpen(false);
    window.scrollTo(0, 0);
  };

  return (
    <ToastProvider>
      <div className="layout">
        <aside className={'sidebar' + (menuOpen ? ' open' : '')}>
          <div className="brand">
            <span className="brand-logo"><Wallet size={20} /></span>
            <span>Finance<br />Manager</span>
            <button className="icon-button menu-close" aria-label="Close menu" onClick={() => setMenuOpen(false)}>
              <X size={20} />
            </button>
          </div>
          <nav>
            {PAGES.map(({ id, label, icon: Icon }) => (
              <button key={id} className={'nav-item' + (id === pageId ? ' active' : '')}
                      aria-current={id === pageId ? 'page' : undefined} onClick={() => open(id)}>
                <Icon size={19} /> {label}
              </button>
            ))}
          </nav>
          <p className="sidebar-note">All calculations run in the C++ program (server.exe).</p>
        </aside>
        {menuOpen && <div className="sidebar-backdrop" onClick={() => setMenuOpen(false)} />}

        <div className="main-area">
          <header className="mobile-bar">
            <button className="icon-button" aria-label="Open menu" onClick={() => setMenuOpen(true)}><Menu size={22} /></button>
            <span className="mobile-title">{current.label}</span>
          </header>
          <main className="content">
            {/* key={pageId}: switching pages creates a fresh page, which reloads its data */}
            <current.Page key={pageId} onNavigate={open} />
          </main>
        </div>
      </div>
    </ToastProvider>
  );
}
