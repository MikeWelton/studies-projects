package cover;

import java.util.ArrayList;

public class FamilyOfSets {
	private ArrayList<Set> sets;
	
	public FamilyOfSets() {
		this.sets = new ArrayList<Set>();
	}
	
	/* Dodaje zbiór do rodziny zbiorów i ustawia mu odpowiedni indeks. */
	public void addSet(Set newSet) {
		sets.add(newSet);
		int index = sets.size();
		newSet.setIndex(index);
	}
	
	public ArrayList<Set> getAllSets() {
		return this.sets;
	}
	
	public Set getSet(int index) {
		return sets.get(index - 1);
	}
	
	public int size() {
		return sets.size();
	}
}
