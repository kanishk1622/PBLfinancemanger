// Small display pieces used on several pages.
import {
  Banknote, Briefcase, Car, Film, Fuel, Gift, GraduationCap, HeartPulse, House,
  Receipt, Shirt, ShoppingBag, Smartphone, Tag, Utensils, Plane, Wallet, Zap,
} from 'lucide-react';

// Picks an icon from words in the category name; anything unknown gets a tag icon.
const ICON_RULES = [
  [/food|grocer|restaurant|snack|lunch|dinner|cafe|tea/i, Utensils],
  [/rent|home|house/i, House],
  [/salary|income|wage/i, Wallet],
  [/bonus|freelance|business|work/i, Briefcase],
  [/travel|trip|flight|holiday/i, Plane],
  [/bill|electric|water|gas|internet/i, Zap],
  [/fuel|petrol|diesel/i, Fuel],
  [/car|bike|bus|metro|taxi|cab|transport/i, Car],
  [/shop|amazon|flipkart/i, ShoppingBag],
  [/cloth|shirt|fashion/i, Shirt],
  [/health|medic|doctor|hospital|gym/i, HeartPulse],
  [/school|college|fee|book|course|education/i, GraduationCap],
  [/movie|film|fun|game|entertain|netflix/i, Film],
  [/gift/i, Gift],
  [/phone|mobile|recharge/i, Smartphone],
  [/tax|emi|loan|insurance/i, Receipt],
  [/cash|atm/i, Banknote],
];

// The same category always gets the same colour (chosen from its letters).
export const PALETTE = ['#5b4cdb', '#0ea5a4', '#f59e0b', '#ec4899', '#3b82f6', '#22c55e', '#f97316', '#8b5cf6', '#14b8a6', '#ef4444'];
export function categoryColor(category) {
  let hash = 0;
  for (const ch of category.toLowerCase()) hash = (hash * 31 + ch.charCodeAt(0)) % 1000;
  return PALETTE[hash % PALETTE.length];
}

export function CategoryIcon({ category, size = 38 }) {
  const Icon = (ICON_RULES.find(([pattern]) => pattern.test(category)) || [null, Tag])[1];
  const color = categoryColor(category);
  return (
    <span className="category-icon" style={{ width: size, height: size, color, background: color + '1f' }}>
      <Icon size={size * 0.5} />
    </span>
  );
}

// value 0-100+; turns red ("over") when the budget is exceeded.
export function ProgressBar({ percent, over = false, color }) {
  return (
    <div className={'progress' + (over ? ' over' : '')}>
      <div style={{ width: Math.min(Math.max(percent, 0), 100) + '%', background: over ? undefined : color }} />
    </div>
  );
}

export function EmptyState({ icon: Icon, title, text, action }) {
  return (
    <div className="empty">
      {Icon && <Icon size={36} className="empty-icon" />}
      <p className="empty-title">{title}</p>
      {text && <p className="empty-text">{text}</p>}
      {action}
    </div>
  );
}

export function PageHeader({ title, subtitle, children }) {
  return (
    <div className="page-header">
      <div>
        <h1>{title}</h1>
        {subtitle && <p className="subtitle">{subtitle}</p>}
      </div>
      {children && <div className="page-actions">{children}</div>}
    </div>
  );
}

// A labelled form field. Extra props (type, min, step, required...) go to the <input>.
export function Field({ label, children, ...inputProps }) {
  return (
    <label className="field">
      <span>{label}</span>
      {children || <input {...inputProps} />}
    </label>
  );
}
