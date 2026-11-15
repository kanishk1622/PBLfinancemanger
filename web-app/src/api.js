// The only place that talks to server.exe. Every rule (balances, budgets, undo,
// hash table, sorting...) is applied by the C++ program; React only shows results.

// GET when there are no params (reads data), POST when there are (changes data).
// The X-Finance-App header proves the request comes from our page (server.cpp checks it: CSRF protection).
export async function api(path, params) {
  const options = { headers: { 'X-Finance-App': '1' } };
  if (params) {
    options.method = 'POST';
    options.body = new URLSearchParams(params);   // form format: amount=500&category=Food
  }

  let response;
  try {
    response = await fetch(path, options);
  } catch {
    throw new Error('Cannot reach the server. Is server.exe still running?');
  }

  let data;
  try {
    data = await response.json();
  } catch {
    throw new Error('The server sent an unexpected reply.');
  }
  if (!data.ok) throw new Error(data.error);
  return data;
}
