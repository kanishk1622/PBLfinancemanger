// After an expense, server.exe says which budget it counts towards (or why none).
// This turns that answer into a toast message.
import { money, monthLabel } from '../format.js';

export function showBudgetToast(toast, data, intro) {
  if (data.budget) {
    const b = data.budget;
    const text = `${intro} ${b.category} budget for ${monthLabel(b.month, b.year)}: ` +
                 `${money(b.spent)} of ${money(b.limit)} spent (${b.percent.toFixed(0)}%).`;
    if (b.remaining < 0) toast(`${text} Over budget by ${money(-b.remaining)}!`, 'warn');
    else toast(text);
  } else if (data.budgetNote) {
    toast(`${intro} Note: ${data.budgetNote}`, 'warn');
  } else {
    toast(intro);
  }
}
