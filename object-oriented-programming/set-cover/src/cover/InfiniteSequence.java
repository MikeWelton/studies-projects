package cover;

import java.util.TreeSet;

public class InfiniteSequence extends Sequence {
	
	public InfiniteSequence(int firstElement, int difference) {
		super(firstElement, difference);
	}
	
	@Override
	public int numberOfCommonElements(TreeSet<Integer> elementsOfCoveredSet) {
		if(!elementsOfCoveredSet.isEmpty()) {
			int counter = 0;
			int i = this.firstElement;
			int last = elementsOfCoveredSet.last();
			while(i <= last) {
				if(elementsOfCoveredSet.remove(i)) {
					counter++;
				}
				i += difference;
			}
			return counter;
		}
		else {
			return 0;
		}
	}
	
	@Override
	public void deleteCommonPartFrom(TreeSet<Integer> elementsOfCoveredSet) {
		if(!elementsOfCoveredSet.isEmpty()) {
			int i = this.firstElement;
			int last = elementsOfCoveredSet.last();
			while(i <= last) {
				elementsOfCoveredSet.remove(i);
				i += difference;
			}
		}
	}
	
	@Override
	public TreeSet<Integer> getCommonPart(TreeSet<Integer> elementsOfCoveredSet) {
		TreeSet<Integer> ret = new TreeSet<Integer>();
		if(!elementsOfCoveredSet.isEmpty()) {
			int i = this.firstElement;
			int last = elementsOfCoveredSet.last();
			while(i <= last) {
				if(elementsOfCoveredSet.remove(i)) {
					ret.add(i);
				}
				i += difference;
			}
		}
		return ret;
	}
}
