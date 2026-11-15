// Small messages in the corner ("Transaction added", errors...). Any page can show one
// with:  const toast = useToast();  toast('Saved');  toast('Error!', 'error');
// A message can have a button, e.g. "Undo" after deleting.
import { createContext, useCallback, useContext, useState } from 'react';
import { CircleCheck, Info, TriangleAlert, X } from 'lucide-react';

const ToastContext = createContext(() => {});

export function useToast() {
  return useContext(ToastContext);
}

let nextId = 1;

export function ToastProvider({ children }) {
  const [toasts, setToasts] = useState([]);

  const remove = useCallback(id => setToasts(list => list.filter(t => t.id !== id)), []);

  // kind: 'ok' | 'warn' | 'error';  action: optional { label, onClick }
  const show = useCallback((text, kind = 'ok', action = null) => {
    const id = nextId++;
    setToasts(list => [...list.slice(-3), { id, text, kind, action }]);   // at most 4 at once
    setTimeout(() => remove(id), kind === 'ok' && !action ? 4000 : 8000);
  }, [remove]);

  const icons = { ok: CircleCheck, warn: TriangleAlert, error: Info };

  return (
    <ToastContext.Provider value={show}>
      {children}
      <div className="toasts" role="status" aria-live="polite">
        {toasts.map(t => {
          const Icon = icons[t.kind];
          return (
            <div key={t.id} className={`toast toast-${t.kind}`}>
              <Icon size={18} className="toast-icon" />
              <span className="toast-text">{t.text}</span>
              {t.action && (
                <button className="toast-action" onClick={() => { remove(t.id); t.action.onClick(); }}>
                  {t.action.label}
                </button>
              )}
              <button className="icon-button" aria-label="Close message" onClick={() => remove(t.id)}>
                <X size={16} />
              </button>
            </div>
          );
        })}
      </div>
    </ToastContext.Provider>
  );
}
