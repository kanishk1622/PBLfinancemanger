// Loads data for a page when it opens, and gives a reload() to call after a change.
//   const { data, error, reload } = useData(() => api('/api/budgets'));
// deps: values that should trigger a fresh load when they change (e.g. a search).
import { useCallback, useEffect, useRef, useState } from 'react';

export function useData(loader, deps = []) {
  const [data, setData] = useState(null);
  const [error, setError] = useState('');
  const loaderRef = useRef(loader);
  loaderRef.current = loader;

  const reload = useCallback(async () => {
    try {
      setData(await loaderRef.current());
      setError('');
    } catch (err) {
      setError(err.message);
    }
  }, []);

  // eslint-disable-next-line react-hooks/exhaustive-deps
  useEffect(() => { reload(); }, [reload, ...deps]);

  return { data, error, reload };
}
